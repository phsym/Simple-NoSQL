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
#include "crypto/crypto.h"
#include "internal.h"

unsigned int last_id = 0;

hashtable_t *cmd_dict;
cmd_t *cmd_id[256];
bool _proto_init = false;

cmd_t commands[] = {
	{"get", 	OP_GET, 	CF_READ|CF_NEED_DB,		1, 	&do_get, 	"Get command"},
	{"put", 	OP_PUT, 	CF_WRITE|CF_NEED_DB, 	2, 	&do_put, 	"Put command"},
	{"set", 	OP_SET, 	CF_WRITE|CF_NEED_DB, 	2, 	&do_set, 	"Set command"},
	{"list", 	OP_LIST, 	CF_READ|CF_NEED_DB, 	0, 	&do_list, 	"List command"},
	{"rmv", 	OP_RMV, 	CF_WRITE|CF_NEED_DB, 	1, 	&do_rmv, 	"Remove command"},
	{"count", 	OP_COUNT, 	CF_READ|CF_NEED_DB, 	0, 	&do_count, 	"Count command"},
	{"digest", 	OP_DIGEST, 	CF_WRITE|CF_NEED_DB, 	3, 	&do_digest,	"Hash digest calculation"},
	{"help", 	OP_HELP, 	CF_NONE, 				0, 	&do_help, 	"Help command"},
	{"quit", 	OP_QUIT, 	CF_NONE, 				0, 	&do_quit, 	"Quit command"},
	{"trace", 	OP_TRACE,	CF_ADMIN, 				1, 	&do_trace, 	"Trace command"},
	{"time", 	OP_TIME, 	CF_NONE, 				0, 	&do_time, 	"Get server time"},
	{"ping", 	OP_PING, 	CF_NONE, 				0, 	&do_ping, 	"Ping server"},
	{"client", 	OP_CLIENT, 	CF_ADMIN, 				0, 	&do_client, "List clients"},
	{"flush", 	OP_FLUSH, 	CF_WRITE|CF_NEED_DB, 	0, 	&do_flush, 	"Flush database"},
	{"db",		OP_DB,		CF_NONE,				1,	&do_db,		"DB selection"},
	{"passwd",	OP_PASSWD,	CF_NONE,				1,	&do_passwd,	"Change user password"},
	{"user",	OP_USER,	CF_ADMIN,				1,	&do_user,	"User account administration"},
	{"dump",	OP_DUMP,	CF_ADMIN|CF_READ,		0,	&do_dump,	"Dump  the current or all databases"}
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
	{
		cmd_t* cmd = cmd_id[req->op];
		if((cmd->flag & CF_NEED_DB) && !req->client->datastore)
		{
			req->reply.message = "No DB selected";
			req->reply.rc = -1;
		}
		else
			cmd->process(req);
	}
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

	char *str;
	char* op = strqtok_r(req, &str);
	if(op == NULL)
		return -1;

	//Convert command to lowercase
	i = 0;
	while(op[i] != '\0')
	{
		op[i] = (char)tolower((int)op[i]);
		i++;
	}
	
	cmd_t *cmd = ht_get(cmd_dict, op);
	
	if(cmd != NULL)
	{
		request->op = cmd->op;
		for(i = 0; i < MAX_ARGC; i++)
		{
			request->argv[i] = strqtok_r(NULL, &str);
			if(request->argv[i] == NULL)
				break;
		}
		request->argc = i;
		if(request->argc < cmd->min_argc)
			return -1;
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
			case OP_CLIENT:
			case OP_DUMP:
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
	req->reply.value = datastore_lookup(req->client->datastore, req->argv[0]);
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
	req->reply.rc = datastore_put(req->client->datastore, req->argv[0], req->argv[1]);
}

void do_set(request_t* req)
{
	req->reply.rc = datastore_set(req->client->datastore, req->argv[0], req->argv[1]);
}

void do_list(request_t* req)
{
	int n = datastore_keys_number(req->client->datastore);
	char* keys[n];
	// TODO : filtering
	datastore_list_keys(req->client->datastore, keys, n);
	size_t size = n*(MAX_KEY_SIZE+3)*sizeof(char);
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
	req->reply.rc = datastore_remove(req->client->datastore, req->argv[0]);
}

void do_count(request_t* req)
{
	req->reply.message = malloc(9);
	snprintf(req->reply.message, 8, "%llu", datastore_keys_number(req->client->datastore));
	req->reply.rc = 0;
}

void do_digest(request_t* req)
{
	//Convert digest algorithm name to lowercase
	int i = 0;
	while(req->argv[0][i] != '\0')
	{
		req->argv[0][i] = tolower((int)req->argv[0][i]);
		i++;
	}
	hash_algo_t* algo = crypto_get_hash_algo(req->argv[0]);
	if(algo == NULL)
	{
		char *error = "Unknown hash algorithm";
		_log(LVL_DEBUG, "%s : %s\n", error, req->argv[0]);
		req->reply.rc = -1;
		req->reply.message = error;
		return;
	}
	char digest_str[algo->digest_str_len];
	crypto_hash_str(algo, req->argv[2], strlen(req->argv[2]), digest_str);
	req->reply.rc = datastore_set(req->client->datastore, req->argv[1], digest_str);
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
		req->argv[0][i] = toupper((int)req->argv[0][i]);
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

void do_client(request_t* req)
{
	server_t* server = req->client->server;
	int i;
	char buff[2048];
	req->reply.message = malloc(1024*server->num_clients);
	req->reply.message[0] = '\0';
	for(i = 0; i < server->max_client; i++)
	{
		client_t* cli = server->clients[i];
		char* its_me = " ";
		if(cli != NULL)
		{
			if(cli == req->client)
				its_me = "*";
			datastore_t* db = cli->datastore;
			char* us = cli->username;
			sprintf(buff, " %s %d : %s:%d u:%s db:%s\r\n", its_me, i, cli->address, cli->port, (us != NULL ? us : "none"), (db != NULL ? db->name : "none"));
			strcat(req->reply.message, buff);
		}
	}
	req->reply.rc = 0;
}

void do_flush(request_t* req)
{
	datastore_clear(req->client->datastore);
	req->reply.rc = 0;
}

void do_passwd(request_t* req)
{
	if(req->client->username != NULL)
		req->reply.rc = intern_set_password(req->client->server->dbs, req->client->username, req->argv[0]);
	else
		req->reply.rc = -1;
}

void do_db(request_t* req)
{
	if(!strcmp(req->argv[0], "use"))
	{
		char* dbname = req->argv[1];
		datastore_t * store = ht_get(req->client->server->dbs->storages, dbname);
		if(store != NULL)
		{
			req->client->datastore = store;
			req->reply.rc = 0;
		}
		else
		{
			req->reply.rc = -1;
			req->reply.message = "DB not found";
		}
	}
	else if(!strcmp(req->argv[0], "create") && (req->argc >= 4))
		req->reply.rc = intern_create_new_db(req->client->server->dbs, req->argv[1], req->argv[2], req->argv[3]);
	else if(!strcmp(req->argv[0], "default"))
		req->reply.rc = intern_set_default_db(req->client->server->dbs, req->argv[1]);
	else
		req->reply.rc = -1;
}

void do_user(request_t* req)
{
	if(!strcmp(req->argv[0], "create") && (req->argc >= 3))
		req->reply.rc = intern_create_user(req->client->server->dbs, req->argv[1], req->argv[2]);
	else
		req->reply.rc = -1;
}

void do_dump(request_t* req)
{
	int n = datastore_keys_number(req->client->datastore);
	char* keys[n];
	datastore_list_keys(req->client->datastore, keys, n);
	size_t size = n*(MAX_KEY_SIZE + MAX_VALUE_SIZE + 10)*sizeof(char);
	size = size>0 ? size : 1;
	req->reply.message = malloc(size);
	memset(req->reply.message, '\0', size);
	int i;
	for(i = 0; i < n; i++)
	{
		char* value = datastore_lookup(req->client->datastore, keys[i]);
		strcat(req->reply.message, "SET ");
		strcat(req->reply.message, keys[i]);
		strcat(req->reply.message, " ");
		strcat(req->reply.message, value);
		strcat(req->reply.message, "\r\n");
	}
	req->reply.rc = 0;
}
