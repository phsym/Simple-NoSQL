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

#ifdef __MINGW32__
	bool WSAinit = false; //Is Winsock Initialized
#endif


server_t* server_create(unsigned int bind_addr, short port, datastore_t* datastore)
{
	server_t* server = malloc(sizeof(server_t));
	server->running = 0;
	server->socket = -1;
	server->port = port;
	server->datastore = datastore;
	server->bind_addr = bind_addr;
	return server;
}

TH_HDL client_handler(void* client)
{
	client_t* cli = (client_t*)client;

	char buff[BUFF_SIZE];
	while(cli->server->running)
	{
		//readline
		int r = read_line(cli->sock, buff, BUFF_SIZE);
		if(r <= 0)
			break;
		request_t req;

		_log(LVL_TRACE, "Request received : %s\n", buff);

		if(decode_request(&req, buff, r) < 0)
			continue;
		// Process request
		process_request(cli->server->datastore, &req);

		//Send response
		encode_reply(&req, buff, BUFF_SIZE);

		_log(LVL_TRACE, "Reply sent : %s\n", buff);
		send(cli->sock, buff, strlen(buff), 0);
	}
	close(cli->sock);

	free(client);

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
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	server->socket = sock;

#ifndef __MINGW32__
	int val = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&val, sizeof(val));
#endif

	struct sockaddr_in addr;
	addr.sin_port=htons(server->port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = server->bind_addr;


	if(bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != 0)
	{
		_perror("Error Bind");
		exit(1);
	}

	if(listen(sock, 1000) != 0)
	{
		_perror("Error listen");
		exit(1);
	}

	struct sockaddr_in addr_client;
	socklen_t socklen = sizeof(struct sockaddr_in);
	_log(LVL_INFO, "Accepting connections\n");
	while(server->running)
	{
		int client_sock = accept(sock, (struct sockaddr*)&addr_client, &socklen);
		if(!server->running)
			break;
		else if(client_sock < 0)
		{
			_perror("Error accept");
		}
		else
		{
			client_t *client = malloc(sizeof(client_t));
			client->server = server;
			client->sock = client_sock;
			thread_create(&(client->thread), &client_handler, client, 1);
			_log(LVL_DEBUG, "New connection from %s:%d\n", inet_ntoa(addr_client.sin_addr),addr_client.sin_port);
		}
	}

	//TODO : wait client

	close(sock);
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
	//TODO : Notify clients
	server->running = false;
	close(server->socket);
	server->socket = -1;
#ifdef __MINGW32__
	//TODO : Cancel Thread
	if(WSAinit)
	{
		WSACleanup();
		WSAinit = false;
	}
#else
	pthread_cancel(server->thread); // TODO: Wait clients end
#endif
}

void server_destroy(server_t* server)
{
	if(server->running)
		server_stop(server);
	free(server);
}

void process_request(datastore_t* datastore, request_t* req)
{
	req->reply.message = "";

	switch(req->op)
	{
		case OP_GET:
			req->reply.value = datastore_lookup(datastore, req->name);
			req->reply.name = req->name;
			if(req->reply.value == NULL)
			{
				req->reply.rc = -1;
				req->reply.message = "Entry not found";
			}
			else
				req->reply.rc = 0;
			break;
		case OP_PUT:
			req->reply.rc = datastore_put(datastore, req->name, req->value);
			break;
		case OP_SET:
			req->reply.rc = datastore_set(datastore, req->name, req->value);
			break;
		case OP_LIST:
			{
				int n = datastore_keys_number(datastore);
				char* keys[n];
				//TODO : filtering
				datastore_list_keys(datastore, keys, n);
				int size = n*(32+1)*sizeof(char);
				req->reply.message = malloc(size);
				memset(req->reply.message, '\0', size);
				int i;
				for(i = 0; i < n; i++)
				{
					strncat(req->reply.message, keys[i], 32);
					strcat(req->reply.message, "\r\n");
				}
				req->reply.rc = 0;
			}
			break;
		case OP_RMV:
			req->reply.rc = datastore_remove(datastore, req->name);
			break;
		default:
			req->reply.message = "Unknown operation";
			req->reply.rc = -1;
			break;
	}
}

int read_line(int sock, char* out, int out_len)
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
		if(buff[0] != '\r')
			out[i++] = buff[0];
	} while(buff[0] != '\n' && buff[0] != '\0' && i < out_len);
	if(i >= out_len)
		return -1;
	return i;
}
