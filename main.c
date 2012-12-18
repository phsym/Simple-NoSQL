/*
 * main.c
 *
 *  Created on: 17 févr. 2012
 *      Author: phsymo10
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "network.h"
#include "datastorage.h"
#include "utils.h"
#include "config.h"

typedef struct {
	volatile bool running;
	server_t* server;
	datastore_t* datastore;
	config_t* config;
}Application;

Application app;

void sig_broken_pipe(int signal)
{
	_log(LVL_WARNING, "Broken pipe\n");
	return;
}

void sig_interrupt(int signal)
{
	_log(LVL_INFO, "Interrupted !!!\n");

	app.running = false;
	_log(LVL_INFO, "Stopping server ...\n");
	server_stop(app.server);
}

int main(int argc, char* argv[])
{
//	int r;
//	for (r = 0; r <= 107; r++)
//		printf ("%d : \033[%d;01mBonjour\033[00m\n", r, r);
//	exit(0);
	_log(LVL_INFO, "Starting server ... \n");

	_log(LVL_INFO, "Loading settings ... \n");
	app.config = malloc(sizeof(config_t));
	config_load(app.config,"config.cfg");

	_log(LVL_DEBUG, "Initializing signal handlers ... \n");
	//Initialize interrupts handlers
	signal(SIGINT,  &sig_interrupt);
	signal(SIGTERM, &sig_interrupt);

#ifndef __MINGW32__
	//These signals don't exists with MinGW
	signal(SIGQUIT, &sig_interrupt);
	signal(SIGPIPE, &sig_broken_pipe);
#endif

	app.running = true;

	_log(LVL_INFO, "Initializing data storage ...\n");
	app.datastore = datastore_create(app.config->storage_size, app.config->index_len);

	_log(LVL_INFO, "Initializing server ...\n");
	app.server = server_create(app.config->bind_address, app.config->bind_port,app.datastore);

	_log(LVL_INFO, "Starting network ...\n");
	server_start(app.server);

	_log(LVL_INFO, "Server is now running on port %d \n\n", app.config->bind_port);
	server_wait_end(app.server);
	server_destroy(app.server);

	datastore_destroy(app.datastore);

	free(app.config);

	_log(LVL_INFO, "Cleaned up\n");

	return 0;
}
