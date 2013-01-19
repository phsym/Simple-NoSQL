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
 * network.h
 *
 *  Created on: 7 juin 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include "concurrency.h"
#include "datastorage.h"
#include "utils.h"

#define BUFF_SIZE 2048

//TODO : Transactions (server side, and client side)



typedef struct server_t{
	bool running;
	thread_t thread;
	int socket;
	short port;
	unsigned int bind_addr;
	int max_client;
	struct client_t **clients;
	bool auth;

	datastore_t* datastore;
}server_t;

typedef struct client_t{
	char address[20]; // IP address
	u_short port;
	bool running;
	thread_t thread;
	struct server_t* server;
	int sock;
}client_t;

server_t* server_create(unsigned int bind_addr, short port, bool auth, datastore_t* datastore, int max_client);

void server_stop(server_t* server);

void server_destroy(server_t* server);

int server_register_cient(server_t* server, client_t* client);

void server_unregister_client(server_t* server, client_t* client);

void stop_client(client_t* client);

bool client_authenticate(client_t* cli);

TH_HDL client_handler(void* client);

TH_HDL server_handler(void* serv);

void server_start(server_t* server);

void server_wait_end(server_t* server);

int read_line(int sock, char* out, int out_len, bool keep_lf);

#endif /* NETWORK_H_ */
