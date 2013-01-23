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
 * protocol.c
 *
 *  Created on: 8 août 2012
 *      Author: Pierre-Henri Symoneaux
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "protocol.h"
#include "utils.h"
#include "crypto.h"

unsigned int last_id = 0;

hashtable_t *cmd_dict;
cmd_t *cmd_id[256];
bool _proto_init = false;

cmd_t commands[] = {
	{"get", OP_GET, FLAG_READ, 1, "Get command", &do_get},
	{"put", OP_PUT, FLAG_WRITE, 2, "Put command", &do_put},
	{"set", OP_SET, FLAG_WRITE, 2, "Set command", &do_set},
	{"list", OP_LIST, FLAG_READ, 0, "List command", &do_list},
	{"rmv", OP_RMV, FLAG_WRITE, 1, "Remove command", &do_rmv},
	{"count", OP_COUNT, FLAG_READ, 0, "Count command", &do_count},
	{"digest", OP_DIGEST, FLAG_WRITE, 3, "Hash digest calculation", &do_digest},
	{"help", OP_HELP, FLAG_NONE, 0, "Help command", &do_help},
	{"quit", OP_QUIT, FLAG_NONE, 0, "Quit command", &do_quit},
	{"trace", OP_TRACE, FLAG_NONE, 1, "Trace command", &do_trace},
	{"time", OP_TIME, FLAG_NONE, 0, "Get server time", &do_time},
	{"ping", OP_PING, FLAG_NONE, 0, "Ping server", &do_ping},
	{"who", OP_WHO, FLAG_NONE, 0, "List clients", &do_who}
};

void protocol_init()
{
	if(!_proto_init)
	{
		int num_cmd = sizeof(commands)/sizeof(cmd_t);
		cmd_dict = ht_create(256);
		int i;
		for(i = 0; i < 256; i++)
			cmd_id[i] = NULL;
		for(i = 0; i < num_cmd; i++)
			register_command(commands+i);
		_proto_init = true;
	}
}

void protocol_cleanup()
{
	if(_proto_init)
	{
		ht_clear(cmd_dict, 0);
		ht_destroy(cmd_dict);
		_proto_init = false;
	}
}

void register_command(cmd_t *cmd)
{
	_log(LVL_DEBUG, "Registering command %s\n", cmd->name);
	ht_put(cmd_dict, cmd->name, cmd);
	cmd_id[cmd->op] = cmd;
}

void process_request(request_t* req)
{
	req->reply.message = "";

	if(cmd_id[req->op] != NULL)
		cmd_id[req->op]->process(req);
	else
	{
		req->reply.message = "Unknown operation";
		req->reply.rc = -1;
	}
	req->reply.replied = true;
}

int decode_request(client_t* client, request_t* request, char* req, int len)
{
	memset(request, 0, sizeof(request));

	request->reply.replied = false;
	request->id = last_id ++;
	request->client = client;

	int i;
	for(i = 0; i < MAX_ARGC; i++)
		request->argv[i] = "";

	char* c = strchr(req, '\n');
	if(c != NULL)
		c[0] = '\0';
	c = strchr(req, '\r');
	if(c != NULL)
		c[0] = '\0';

	char *str[len];
	char* op = strtok_r(req, " ", str);
	if(op == NULL)
		return -1;

	//Convert command to lowercase
	i = 0;
	while(op[i] != '\0')
	{
		op[i] = tolower(op[i]);
		i++;
	}
	
	cmd_t *cmd = ht_get(cmd_dict, op);
	
	if(cmd != NULL)
	{
		request->op = cmd->op;
		request->argc = cmd->argc;
		for(i = 0; i < MIN(cmd->argc, MAX_ARGC); i++)
		{
			request->argv[i] = strtok_r(NULL, " ", str);
			if(request->argv[i] == NULL)
				return -1;
		}
	}
	else
		return -1;
	
	return 0;
}

void encode_reply(request_t* req, char* buff, int buff_len)
{
	buff[0] = '\0';
	if(req->reply.rc == 0)
	{
		strcat(buff, "OK\r\n");
		switch(req->op)
		{
			case OP_GET:
				strcat(buff, req->reply.value);
				strcat(buff, "\r\n");
				break;
			case OP_HELP:
			case OP_COUNT:
			case OP_TIME:
			case OP_WHO:
			case OP_LIST:
				strcat(buff, req->reply.message);
				strcat(buff, "\r\n");
				free(req->reply.message);
				break;
			case OP_PING:
			case OP_QUIT:
				strcat(buff, req->reply.message);
				strcat(buff, "\r\n");
				break;
			default:
				break;
		}
	}
	else
	{
		strcat(buff, "KO\r\n");
		strcat(buff,  req->reply.message);
		strcat(buff, "\r\n");
	}
}

void do_get(request_t* req)
{
	req->reply.value = datastore_lookup(req->client->server->datastore, req->argv[0]);
	req->reply.name = req->argv[0];
	if(req->reply.value == NULL)
	{
		req->reply.rc = -1;
		req->reply.message = "Entry not found";
	}
	else
		req->reply.rc = 0;
}

void do_put(request_t* req)
{
	req->reply.rc = datastore_put(req->client->server->datastore, req->argv[0], req->argv[1]);
}

void do_set(request_t* req)
{
	req->reply.rc = datastore_set(req->client->server->datastore, req->argv[0], req->argv[1]);
}

void do_list(request_t* req)
{
	int n = datastore_keys_number(req->client->server->datastore);
	char* keys[n];
	// TODO : filtering
	datastore_list_keys(req->client->server->datastore, keys, n);
	size_t size = n*(MAX_KEY_SIZE+1)*sizeof(char);
	size = size>0 ? size : 1;
	req->reply.message = malloc(size);
	memset(req->reply.message, '\0', size);
	int i;
	for(i = 0; i < n; i++)
	{
		strncat(req->reply.message, keys[i], MAX_KEY_SIZE);
		strcat(req->reply.message, "\r\n");
	}
	req->reply.rc = 0;
}

void do_rmv(request_t* req)
{
	req->reply.rc = datastore_remove(req->client->server->datastore, req->argv[0]);
}

void do_count(request_t* req)
{
	req->reply.message = malloc(9);
	snprintf(req->reply.message, 8, "%d", datastore_keys_number(req->client->server->datastore));
	req->reply.rc = 0;
}

void do_digest(request_t* req)
{
	//Convert digest algorithm name to lowercase
	int i = 0;
	while(req->argv[0][i] != '\0')
	{
		req->argv[0][i] = tolower(req->argv[0][i]);
		i++;
	}
	hash_algo_t* algo = crypto_get_hash_algo(req->argv[0]);
	if(algo == NULL)
	{
		char *error = "Unknown hash algorithm";
		_log(LVL_DEBUG, "%s : %s", error, req->argv[0]);
		req->reply.rc = -1;
		req->reply.message = error;
		return;
	}
	char digest_str[algo->digest_str_len];
	crypto_hash_str(algo, req->argv[2], strlen(req->argv[2]), digest_str);
	req->reply.rc = datastore_set(req->client->server->datastore, req->argv[1], digest_str);
}

void do_help(request_t* req)
{
	int n = cmd_dict->e_num;
	char* keys[n];
	ht_list_keys(cmd_dict, keys, n);
	size_t size = n*(MAX_KEY_SIZE+1)*sizeof(char);
	size = size>0 ? size : 1;
	req->reply.message = malloc(size);
	memset(req->reply.message, '\0', size);
	int i;
	for(i = 0; i < n; i++)
	{
		strncat(req->reply.message, keys[i], MAX_KEY_SIZE);
		strcat(req->reply.message, "\r\n");
	}
	req->reply.rc = 0;
}

void do_quit(request_t* req)
{
	req->client->running = false;
	req->reply.rc = 0;
	req->reply.message = "GoodBye";
}

void do_trace(request_t* req)
{
	//Make log level case unsensitive
	int i = 0;
	while(req->argv[0][i] != '\0')
	{
		req->argv[0][i] = toupper(req->argv[0][i]);
		i++;
	}
	for(i = 0; i < MAX_DEBUG_LEVEL; i++)
	{
		if(!strcmp(req->argv[0], DBG_LVL_STR[i]))
		{
			_log(LVL_INFO, "Changing trace level to %s\n", DBG_LVL_STR[i]);
			DEBUG_LEVEL = i;
			req->reply.rc = 0;
			return;
		}
	}
	req->reply.message = "Invalid trace level";
	req->reply.rc = -1;
	return;
}

void do_time(request_t* req)
{
	req->reply.message = malloc(TIME_STRLEN);
	get_current_time_string(req->reply.message, TIME_STRLEN);
	req->reply.rc = 0;
}

void do_ping(request_t* req)
{
	req-> reply.message = "PONG";
	req->reply.rc = 0;
}

void do_who(request_t* req)
{
	server_t* server = req->client->server;
	int i;
	char buff[1024];
	req->reply.message = malloc(1024*server->num_clients);
	req->reply.message[0] = '\0';
	for(i = 0; i < server->max_client; i++)
	{
		client_t* cli = server->clients[i];
		char* its_me = "";
		if(cli != NULL)
		{
			if(cli == req->client)
				its_me = " *";
			sprintf(buff, "%d : %s:%d%s\r\n", i, cli->address, cli->port, its_me);
			strcat(req->reply.message, buff);
		}
	}
	req->reply.rc = 0;
}
