/*
* Simple-NoSQL
* Copyright (C) 2012 Pierre-Henri Symoneaux
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 US
*/

/*
 * network.c
 *
 *  Created on: 7 juin 2012
 *      Author: Pierre-Henri Symoneaux
 */


#ifdef __MINGW32__
	#include <winsock2.h>
	#define socklen_t int // winsock doesn't have socklen_t
	#define SHUT_WR SD_BOTH // winsock doesn't have SHUT_WR
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "network.h"
#include "utils.h"
#include "protocol.h"
#include "crypto.h"
#include "internal.h"

#ifdef __MINGW32__
	bool WSAinit = false; //Is Winsock Initialized
#endif


server_t* server_create(unsigned int bind_addr, short port, bool auth, datastore_t* intern_db, hashtable_t* storages, int max_client)
{
	server_t* server = malloc(sizeof(server_t));
	if(server == NULL)
		return NULL;
	server->running = false;
	server->socket = -1;
	server->port = port;
	server->storages = storages;
	server->intern_db = intern_db;
	server->bind_addr = bind_addr;
	server->auth = auth;
	server->max_client = max_client;
	server->num_clients = 0;
	server->clients = malloc(max_client*sizeof(client_t**));
	int i;
	for(i = 0; i < max_client; i++)
		*(server->clients + i) = NULL;
	return server;
}

client_t* client_create(server_t* server, int sock, char* address, u_short port)
{
	client_t *client = malloc(sizeof(client_t));
	if(client == NULL)
		return NULL;
	client->server = server;
	client->sock = sock;
	client->running = false;
	client->port = port;
	client->trans_open = false;
	strncpy(client->address, address, 20);
	if(server_register_client(server, client) < 0)
	{
		_log(LVL_WARNING, "Max number of active connections reached : %d\n", server->max_client);
		stop_client(client);
		free(client);
		return NULL;
	}

	client->datastore = intern_get_default_db(client->server->intern_db, client->server->storages);
	return client;
}

int server_register_client(server_t* server, client_t* client)
{
	if(server->num_clients >= server->max_client)
		return -1;
	int i;
	for(i = 0; i < server->max_client; i++)
	{
			if (*(server->clients + i) == NULL)
			{
				_log(LVL_TRACE, "Registering client on server\n");
				*(server->clients + i) = client;
				server->num_clients++;
				return i;
			}
	}
	return -1;
}

void server_unregister_client(server_t* server, client_t* client)
{
	if(server->num_clients <= 0)
		return;
	int i;
	for(i = 0; i < server->max_client; i++)
	{
		if(*(server->clients + i) == client)
		{
			_log(LVL_TRACE, "Unregistering client from server\n");
			*(server->clients + i) = NULL;
			server->num_clients--;
		}
	}
}

bool client_authenticate(client_t* cli)
{
	_log(LVL_DEBUG, "Asking authentication\n");
	char* auth_tok = datastore_lookup(cli->server->intern_db, INT_USER_HASH);
	if(auth_tok == NULL)
	{
		_log(LVL_WARNING, "Authentication is activated, but no password has been set. Skipping authentication.\n");
		return true;
	}
	else
	{
		_log(LVL_TRACE, "Stored hash : %s\n", auth_tok);
		char* r = "Authentication needed\r\n";
		_log(LVL_DEBUG, "%s", r);
		send(cli->sock, r, strlen(r), 0);
		//Authenticate user
		char username[32];
		char pass[32];
		char cat[128];
		
		if(read_line(cli->sock, username, 32, false) <= 0)
			return false;
		if(read_line(cli->sock, pass, 32, false) <= 0)
			return false;
		
		strcat(cat, username);
		strcat(cat, ":");
		strcat(cat, pass);
		CAT4(cat, username, PASSWD_SALT, pass);
		
		hash_algo_t* algo = crypto_get_hash_algo("sha256");
		char digest_str[algo->digest_str_len];
		crypto_hash_str(algo, cat, strlen(cat), digest_str);
		
		_log(LVL_TRACE, "Auth token : %s\n", digest_str);
		
		if(!strcmp(digest_str, auth_tok))
		{
			r = "Authentication success\r\n";
			_log(LVL_DEBUG, r);
			send(cli->sock, r, strlen(r), 0);
			return true;
		}
		else
		{
			r = "Authentication failed\r\n";
			_log(LVL_ERROR, r);
			send(cli->sock, r, strlen(r), 0);
			return false;
		}
	}
}

TH_HDL client_handler(void* client)
{
	client_t* cli = (client_t*)client;
	char buff[BUFF_SIZE];

	cli->running = true;
	if(cli->server->auth && !client_authenticate(cli))
			cli->running = false;
	
	while(cli->running && cli->server->running)
	{
		//readline
		int r = read_line(cli->sock, buff, BUFF_SIZE, true);
		if(r <= 0)
			break;
		request_t req;

		_log(LVL_TRACE, "Request received : %s\n", buff);

		if(decode_request(cli, &req, buff, r) < 0)
		{
			send(cli->sock, "KO\r\n", 4, 0);
			continue;
		}
		// Process request
		process_request(&req);

		//Send response
		encode_reply(&req, buff, BUFF_SIZE);

		_log(LVL_TRACE, "Reply sent : %s\n", buff);
		send(cli->sock, buff, strlen(buff), 0);
	}

	_log(LVL_TRACE, "Exiting client thread\n", buff);

	cli->running = false;
	shutdown(cli->sock, SHUT_WR);
	close(cli->sock);
	server_unregister_client(cli->server, cli);
	free(client);

	_log(LVL_TRACE, "Client thread exited\n", buff);
	TH_RETURN;
}

TH_HDL server_handler(void* serv)
{
	server_t* server = (server_t*)serv;
	server->running = true;

#ifdef __MINGW32__
	if(!WSAinit)
	{
		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(2,2), &wsadata) == SOCKET_ERROR) {
			_log(LVL_FATAL, "Error creating socket.");
			TH_RETURN;
		}
		WSAinit = true;
	}
#endif
	
	server->socket = socket(AF_INET, SOCK_STREAM, 0);

#ifndef __MINGW32__
	int val = 1;
	setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, (void*)&val, sizeof(val));
#endif

	struct sockaddr_in addr;
	addr.sin_port=htons(server->port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = server->bind_addr;


	if(bind(server->socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != 0)
	{
		_perror("Error Bind");
		TH_RETURN;
	}

	if(listen(server->socket, server->max_client) != 0)
	{
		_perror("Error listen");
		TH_RETURN;
	}

	struct sockaddr_in addr_client;
	socklen_t socklen = sizeof(struct sockaddr_in);
	_log(LVL_INFO, "Accepting connections\n");
	while(server->running)
	{
		int client_sock = accept(server->socket, (struct sockaddr*)&addr_client, &socklen);
		if(!server->running)
			break;
		else if(client_sock < 0)
			_perror("Error accept");
		else
		{
			client_t *client = client_create(server, client_sock, inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));
			if(client == NULL)
				continue;
			_log(LVL_DEBUG, "New connection from %s:%d\n", inet_ntoa(addr_client.sin_addr), client->port);
			thread_create(&(client->thread), &client_handler, client, 1);		
		}
	}

	//Wait clients end
	int i;
	for(i = 0; i < server->max_client; i++)
	{
		if(server->clients[i] != NULL)
			thread_join(&(server->clients[i]->thread));
	}
	close(server->socket);
	TH_RETURN;
}

void server_start(server_t *server)
{
	thread_create(&(server->thread), &server_handler, server, 0);
}

void server_wait_end(server_t* server)
{
	thread_join(&(server->thread));
}

void server_stop(server_t* server)
{
	//Stop server
	server->running = false;
	close(server->socket);
	server->socket = -1;
	// Stop clients
	int i;
	for(i = 0; i < server->max_client; i++)
	{
		if(server->clients[i] != NULL)
			stop_client(server->clients[i]);
	}
#ifdef __MINGW32__
	//TODO : Cancel Thread
	if(WSAinit)
	{
		WSACleanup();
		WSAinit = false;
	}
#else
	pthread_cancel(server->thread);
#endif
}

void stop_client(client_t* client)
{
	client->running = false;
	shutdown(client->sock, SHUT_WR);
	close(client->sock);
}

void server_destroy(server_t* server)
{
	if(server->running)
		server_stop(server);
	free(server->clients);
	free(server);
}

int read_line(int sock, char* out, int out_len, bool keep_lf)
{
	char buff[1];
	memset(out, 0, out_len);
	//readline
	int r = 0;
	int i = 0;
	do {
		r = recv(sock, buff, 1, 0);
		if(r <=0)
			break;
		if(!keep_lf && buff[0] == '\n')
			break;
		if(buff[0] != '\r')
			out[i++] = buff[0];
	} while(buff[0] != '\n' && buff[0] != '\0' && i < out_len);
	if(i >= out_len)
		return -1;
	return i;
}
