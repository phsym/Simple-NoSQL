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
	{"client", 	OP_CLIENT, 	CF_ADMIN, 				0, 	&do_client, 	"List clients"},
	{"flush", 	OP_FLUSH, 	CF_WRITE|CF_NEED_DB, 	0, 	&do_flush, 	"Flush database"},
	{"db",		OP_DB,		CF_NONE,				1,	&do_db,		"DB selection"},
	{"passwd",	OP_PASSWD,	CF_NONE,				2,	&do_passwd,	"Change user password"}
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
		op[i] = tolower(op[i]);
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
		if(request->argc < cmd->argc)
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
		req->argv[0][i] = tolower(req->argv[0][i]);
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
			sprintf(buff, " %s %d : %s:%d db:%s\r\n", its_me, i, cli->address, cli->port, (db ? db->name : "none"));
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
	char cat[128];
	cat[0] = '\0';

	strcat(cat, req->argv[0]);
	strcat(cat, ":");
	strcat(cat, req->argv[1]);

	hash_algo_t* algo = crypto_get_hash_algo("sha256");
	char digest_str[algo->digest_str_len];
	crypto_hash_str(algo, cat, strlen(cat), digest_str);
	datastore_set(req->client->server->intern_db, "DB_ADM.USER.AUTH_HASH", digest_str);
	req->reply.rc = 0;
	_log(LVL_INFO, "Password changed\n");
}

void do_db(request_t* req)
{
	if(!strcmp(req->argv[0], "use"))
	{
		char* dbname = req->argv[1];
		datastore_t * store = ht_get(req->client->server->storages, dbname);
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
	else if(!strcmp(req->argv[0], "create"))
	{
		if(req->argc < 3)
		{
			req->reply.rc = -1;
			return;
		}
		char* dbname = req->argv[1];
		datastore_t * store = ht_get(req->client->server->storages, dbname);
		if(store != NULL)
		{
			req->reply.rc = -1;
			req->reply.message = "DB already exists";
		}
		else
		{
			store = datastore_create(req->argv[1], strtoul(req->argv[2], NULL, 10), strtoul(req->argv[3], NULL, 10));
			if(store != NULL)
			{
				_log(LVL_INFO, "Creating db %s\n", dbname);
				char* db_names = datastore_lookup(req->client->server->intern_db, "DATABASES");
				if(db_names == NULL)
				{
					datastore_put(req->client->server->intern_db, "DATABASES", "");
					db_names = datastore_lookup(req->client->server->intern_db, "DATABASES");
				}
				strcat(db_names, " ");
				strcat(db_names, dbname);

				char tmp_k[2048];

				tmp_k[0] = '\0';
				strcat(tmp_k, "DB.");
				strcat(tmp_k, dbname);
				strcat(tmp_k, ".STORAGE_SIZE");
				datastore_put(req->client->server->intern_db, tmp_k, req->argv[2]);

				tmp_k[0] = '\0';
				strcat(tmp_k, "DB.");
				strcat(tmp_k, dbname);
				strcat(tmp_k, ".INDEX_SIZE");
				datastore_put(req->client->server->intern_db, tmp_k, req->argv[3]);

				ht_put(req->client->server->storages, dbname, store);
				req->reply.rc = 0;
			}
			else
			{
				req->reply.rc = -1;
			}
		}
	}
	else if(!strcmp(req->argv[0], "default"))
	{
		char* dbname = req->argv[1];
		datastore_t * store = ht_get(req->client->server->storages, dbname);
		if(store != NULL)
		{
			req->client->datastore = store;
			datastore_put(req->client->server->intern_db, "DEFAULTDB", dbname);
			req->reply.rc = 0;
			_log(LVL_INFO, "Default databased changed to %s\n", dbname);
		}
		else
		{
			req->reply.rc = -1;
			req->reply.message = "DB not found";
		}
	}
	else
		req->reply.rc = -1;
}
