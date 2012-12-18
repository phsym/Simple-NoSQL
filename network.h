/*
 * network.h
 *
 *  Created on: 7 juin 2012
 *      Author: phsymo10
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include "concurrency.h"
#include "protocol.h"
#include "datastorage.h"
#include "utils.h"

#define BUFF_SIZE 2048

//TODO : Transactions (server side, and client side)

typedef struct {
	volatile bool running;
	thread_t thread;
	int socket;
	short port;
	unsigned int bind_addr;
	datastore_t* datastore;
}server_t;

typedef struct {
	thread_t thread;
	server_t* server;
	int sock;
}client_t;

server_t* server_create(unsigned int bind_addr, short port, datastore_t* datastore);

void server_stop(server_t* server);

void server_destroy(server_t* server);

TH_HDL client_handler(void* client);

TH_HDL server_handler(void* serv);

void server_start(server_t* server);

void server_wait_end(server_t* server);

void process_request(datastore_t* datastore, request_t* req);

int read_line(int sock, char* out, int out_len);

#endif /* NETWORK_H_ */
