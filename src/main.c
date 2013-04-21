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

#ifdef __MINGW32__
	#include <process.h>
	typedef int pid_t;
	#define _getpid() getpid()
#else
	#include <unistd.h>
	#include <sys/wait.h>
#endif

#include "network.h"
#include "protocol.h"
#include "datastorage.h"
#include "utils.h"
#include "config.h"
#include "crypto/crypto.h"
#include "containers.h"
#include "internal.h"

typedef struct {
	bool running;
	server_t* server;
	dbs_t dbs;
	config_t* config;
}Application;

typedef struct{
	char* config_file;
	char* log_file;
	bool daemon;
	bool angel;
}Options;

enum opt_type {
	BOOLEAN, TEXT, HELP
};

typedef struct Opt {
	char* opt;
	char* help;
	enum opt_type type;
	char* arg_name;
	void* var;
}Opt;

Application app;
Options opt;

bool i_am_angel;
pid_t child_pid;
bool angel_running;

const Opt opt_list[] = {
		{"-c", "Specify the config file to use (default is ./config.cfg)", TEXT, "config_file", &(opt.config_file)},
		{"-l", "Specify the file to log messages in (default is stdout)", TEXT, "log_file", &(opt.log_file)},
#ifndef __MINGW32__
		{"-d", "Daemonize process", BOOLEAN, NULL, &opt.daemon},
		{"-a", "Start an angel process", BOOLEAN, NULL, &opt.angel},
#endif
		{"-h", "Print this help", HELP, NULL, NULL}
};

void sig_broken_pipe(int signal)
{
	if(!i_am_angel)
		_log(LVL_WARNING, "Broken pipe (signal %d)\n", signal);
	return;
}

void sig_interrupt(int signal)
{
	if(i_am_angel)
	{
		_log(LVL_INFO, "Angel interrupted by signal. Relaying to child %d\n", child_pid);
		angel_running = false;
#ifndef __MINGW32__
		kill(child_pid, signal);
#endif
	}
	else
	{
		_log(LVL_INFO, "Interrupted by signal %d\n", signal);
		app.running = false;
		_log(LVL_INFO, "Stopping server ...\n");
		server_stop(app.server);
	}
}

void init_signals_handler()
{
	_log(LVL_DEBUG, "Initializing signal handlers ... \n");
	//Initialize interrupts handlers
	signal(SIGINT,  &sig_interrupt);
	signal(SIGTERM, &sig_interrupt);

#ifndef __MINGW32__
	//These signals don't exists with MinGW
	signal(SIGQUIT, &sig_interrupt);
	signal(SIGPIPE, &sig_broken_pipe);
#endif
}

void daemonize()
{
#ifndef __MINGW32__
	_log(LVL_INFO, "Daemonizing ...\n");
	if(fork() != 0)
		exit(0);
#else
	_log(LVL_WARNING, "Cannot daemonize in Windows\n");
#endif
}

void angelize()
{
#ifndef __MINGW32__
	i_am_angel = true;
	angel_running = true;
	_log(LVL_INFO, "Angel started with PID %d\n", getpid());
	while(angel_running && ((child_pid = fork()) != 0))
	{
		int status;
		waitpid(child_pid, &status, 0);
		if(WIFSIGNALED(status) && WCOREDUMP(status))
		{
			_log(LVL_FATAL, "\n");
			_log(LVL_FATAL, "#######################\n");
			_log(LVL_FATAL, "# !!! CORE DUMPED !!! #\n");
			_log(LVL_FATAL, "#######################\n");
			_log(LVL_FATAL, "\n");
		}
	}
	if(child_pid == 0)
		i_am_angel = false;
	else
		exit(0);
#else
	_log(LVL_WARNING, "Cannot angelize in Windows\n");
#endif
}

void usage(char* bin_name)
{
	int i;
	int n = sizeof(opt_list) / sizeof(Opt);
	printf("\nUSAGE : %s [options]\n\n", bin_name);
	printf("Options :\n");
	for(i = 0; i < n; i++)
	{
		Opt opt = opt_list[i];
		printf("\t %s ", opt.opt);
		if(opt.arg_name != NULL)
			printf("<%s> ", opt.arg_name);
		if(opt.help != NULL)
			printf(": %s ", opt.help);
		printf("\n");
	}
	printf("\n");
	exit(-1);
}

void parse_arguments (int argc, char* argv[])
{
	// Default values
	opt.config_file = "config.cfg";
	opt.log_file = NULL;
	opt.daemon = false;
	opt.angel = false;

	// Parse command line arguments
	if(argc > 1)
	{
		int i, j;
		int n = sizeof(opt_list) / sizeof(Opt);
		for(i = 1; i < argc; i++)
		{
			char* arg = argv[i];
			for(j = 0; j < n; j++)
			{
				if(strcmp(opt_list[j].opt, arg) == 0)
				{
					Opt opt = opt_list[j];
					switch(opt.type)
					{
					case BOOLEAN:
						*((bool*)opt.var) = true;
						break;
					case TEXT:
						if((opt.var != NULL) && (i < argc -1 ))
							*((char**)opt.var) = argv[++i];
						break;
					case HELP:
						usage(argv[0]);
						break;
					}
					break;
				}
			}
			if(j >= n)
			{
				fprintf(stderr, "Wrong argument %s\n", argv[i]);
				usage(argv[0]);
			}
		}
	}
}

int main(int argc, char* argv[])
{
//	int r;
//	for (r = 0; r <= 107; r++)
//		printf ("%d : \033[%d;01mBonjour\033[00m\n", r, r);
//	exit(0);

	parse_arguments(argc, argv);

	_log_init(opt.log_file);
	init_signals_handler();

	if(opt.daemon)
		daemonize();
	if(opt.angel)
		angelize();


	_log(LVL_INFO, "\nStarting server ... \n");

	_log(LVL_INFO, "Application's PID is %d\n", getpid());

	_log(LVL_INFO, "Loading settings ... \n");
	app.config = malloc(sizeof(config_t));
	config_load(app.config, opt.config_file);

	app.running = true;

	_log(LVL_DEBUG, "Initializing crypto ...\n");
	crypto_init();

	_log(LVL_DEBUG, "Initializing protocol ...\n");
	protocol_init();

	_log(LVL_INFO, "Initializing data storage ...\n");
	app.dbs.storages = ht_create(256);
	//Load or create admin DB
	app.dbs.intern_db = datastore_create("internal_db", 1024*1024, 1024*1024);
	//Load existing tables
	intern_load_storages(&app.dbs);

	_log(LVL_INFO, "Initializing server ...\n");
	app.server = server_create(app.config->bind_address, app.config->bind_port, app.config->auth, &app.dbs, app.config->max_clients);

	_log(LVL_INFO, "Starting network ...\n");
	server_start(app.server);

	_log(LVL_INFO, "Server is now running on port %d \n\n", app.config->bind_port);
	server_wait_end(app.server);
	server_destroy(app.server);

	_log(LVL_DEBUG, "Destroy datastores\n");
	datastore_destroy(app.dbs.intern_db);
	hash_elem_it it = HT_ITERATOR(app.dbs.storages);
	char* e;
	while((e = ht_iterate_keys(&it)) != NULL)
		datastore_destroy(ht_remove(app.dbs.storages, e));
	ht_destroy(app.dbs.storages);

	free(app.config);

	_log(LVL_INFO, "Cleaned up\n");

	return 0;
}
