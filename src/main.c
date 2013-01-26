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
 * main.c
 *
 *  Created on: 17 févr. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "network.h"
#include "protocol.h"
#include "datastorage.h"
#include "utils.h"
#include "config.h"
#include "crypto.h"
#include "containers.h"

typedef struct {
	bool running;
	server_t* server;
	hashtable_t* storages;
	datastore_t* intern_db;
	config_t* config;
}Application;

Application app;

void sig_broken_pipe(int signal)
{
	_log(LVL_WARNING, "Broken pipe (signal %d)\n", signal);
	return;
}

void sig_interrupt(int signal)
{
	_log(LVL_INFO, "Interrupted by signal %d\n", signal);

	app.running = false;
	_log(LVL_INFO, "Stopping server ...\n");
	server_stop(app.server);
}

void usage(char* bin_name)
{
	printf("\nUSAGE : %s [options]\n\n", bin_name);
	printf("Options :\n");
	printf("\t -c <config_file> : Specify the config file to use (default is ./config.cfg)\n");
	printf("\t -h : Print this help\n");
	printf("\n");
	exit(-1);
}

int main(int argc, char* argv[])
{
//	int r;
//	for (r = 0; r <= 107; r++)
//		printf ("%d : \033[%d;01mBonjour\033[00m\n", r, r);
//	exit(0);

	char* config_file = "config.cfg";

	if(argc > 1)
	{
		int i;
		for (i = 1; i < argc; i++)
		{
			if((strcmp(argv[i], "-c") == 0) && (i < argc -1))
				config_file = argv[++i];
			else if(strcmp(argv[i], "-h") == 0)
				usage(argv[0]);
		}
	}

	_log(LVL_INFO, "Starting server ... \n");

	_log(LVL_INFO, "Loading settings ... \n");
	app.config = malloc(sizeof(config_t));
	config_load(app.config, config_file);

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

	_log(LVL_DEBUG, "Initializing crypto ...\n");
	crypto_init();

	_log(LVL_DEBUG, "Initializing protocol ...\n");
	protocol_init();

	_log(LVL_INFO, "Initializing data storage ...\n");
	app.storages = ht_create(256);
	//Load or create admin DB
	app.intern_db = datastore_create("internal_db", 1024*1024, 1024*1024);

	char* dbs = datastore_lookup(app.intern_db, "DATABASES");
	char* str;
	char* db = strtok_r(dbs, " ", &str);
	while(db != NULL)
	{
		_log(LVL_INFO, "Loading DB %s\n", db);
		//TODO : Save storage and index size in internal db, and use them here
		datastore_t* store = datastore_create(db, 1024*1024, 1024*1024);
		if(store != NULL)
			ht_put(app.storages, store->name, store);
		db = strtok_r(NULL, " ", &str);
	}

	_log(LVL_INFO, "Initializing server ...\n");
	app.server = server_create(app.config->bind_address, app.config->bind_port, app.config->auth, app.intern_db, app.storages, app.config->max_clients);

	_log(LVL_INFO, "Starting network ...\n");
	server_start(app.server);

	_log(LVL_INFO, "Server is now running on port %d \n\n", app.config->bind_port);
	server_wait_end(app.server);
	server_destroy(app.server);

	_log(LVL_DEBUG, "Protocol cleanup\n");
	protocol_cleanup();
	_log(LVL_DEBUG, "Crypto cleanup\n");
	crypto_cleanup();

	_log(LVL_DEBUG, "Destroy datastores\n");
	datastore_destroy(app.intern_db);
	hash_elem_it it = HT_ITERATOR(app.storages);
	char* e;
	while((e = ht_iterate_keys(&it)) != NULL)
		datastore_destroy(ht_remove(app.storages, e));
	ht_destroy(app.storages);

	free(app.config);

	_log(LVL_INFO, "Cleaned up\n");

	return 0;
}
