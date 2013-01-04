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
#include <string.h>
#include <ctype.h>

#include "protocol.h"
#include "utils.h"

#include "md5.h"
#include "sha1.h"

unsigned int last_id = 0;

index_table_t *cmd_dict;
cmd_t *cmd_id[256];
bool initi = false;

cmd_t commands[] = {
	{"get", OP_GET, FLAG_READ, 1, "Get command", &do_get},
	{"put", OP_PUT, FLAG_WRITE, 2, "Put command", &do_put},
	{"set", OP_SET, FLAG_WRITE, 2, "Set command", &do_set},
	{"list", OP_LIST, FLAG_READ, 0, "List command", &do_list},
	{"rmv", OP_RMV, FLAG_WRITE, 1, "Rmv command", &do_rmv},
	{"md5", OP_MD5, FLAG_WRITE, 2, "Md5 command", &do_md5},
	{"sha1", OP_SHA1, FLAG_WRITE, 2, "Sha1 command", &do_sha1}
};

void init()
{
	if(!initi)
	{
		int num_cmd = sizeof(commands)/sizeof(cmd_t);
		cmd_dict = index_table_create(256);
		int i;
		for(i = 0; i < 256; i++)
			cmd_id[i] = NULL;
		for(i = 0; i < num_cmd; i++)
			register_command(commands+i);
		initi = true;
	}
}

void register_command(cmd_t *cmd)
{
	_log(LVL_DEBUG, "Registering command %s\n", cmd->name);
	index_table_put(cmd_dict, cmd->name, cmd);
	cmd_id[cmd->op] = cmd;
}

void process_request(datastore_t* datastore, request_t* req)
{
	init();
	req->reply.message = "";

	if(strlen(req->name) > MAX_KEY_SIZE)
		req->name[MAX_KEY_SIZE] = '\0';
	if(strlen(req->value) > MAX_VALUE_SIZE)
		req->value[MAX_VALUE_SIZE] = '\0';
			
	if(cmd_id[req->op] != NULL)
		cmd_id[req->op]->process(datastore, req);
	else
	{
		req->reply.message = "Unknown operation";
		req->reply.rc = -1;
	}
}

int decode_request(request_t* request, char* req, int len)
{
	init();
	memset(request, 0, sizeof(request));

	request->reply.replied = 0;
	request->id = last_id ++;
	request->name = "";
	request->value = "";

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
	int i = 0;
	while(op[i] != '\0')
	{
		op[i] = tolower(op[i]);
		i++;
	}
	
	int t = index_table_get(cmd_dict, op);
	
	if(t != -1) // TODO: Adapt indextable, -1 is not a good value for this case
	{
		cmd_t *cmd = t;
		
		request->op = cmd->op;
		if(cmd->argc > 0)
		{
			request->name = strtok_r(NULL, " ", str);
			if(request->name == NULL)
				return -1;
		}
		if(cmd->argc > 1)
		{
			request->value = strtok_r(NULL, " ", str);
			if(request->value == NULL)
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
			case OP_LIST:
				strcat(buff, req->reply.message);
				free(req->reply.message);
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

void do_get(datastore_t* datastore, request_t* req)
{
	req->reply.value = datastore_lookup(datastore, req->name);
	req->reply.name = req->name;
	if(req->reply.value == NULL)
	{
		req->reply.rc = -1;
		req->reply.message = "Entry not found";
	}
	else
		req->reply.rc = 0;
}

void do_put(datastore_t* datastore, request_t* req)
{
	req->reply.rc = datastore_put(datastore, req->name, req->value);
}

void do_set(datastore_t* datastore, request_t* req)
{
	req->reply.rc = datastore_set(datastore, req->name, req->value);
}

void do_list(datastore_t* datastore, request_t* req)
{
	int n = datastore_keys_number(datastore);
	if(n > 0)
	{
		char* keys[n];
		// TODO : filtering
		datastore_list_keys(datastore, keys, n);
		size_t size = n*(MAX_KEY_SIZE+1)*sizeof(char);
		req->reply.message = malloc(size);
		memset(req->reply.message, '\0', size);
		int i;
		for(i = 0; i < n; i++)
		{
			strncat(req->reply.message, keys[i], MAX_KEY_SIZE);
			strcat(req->reply.message, "\r\n");
		}
	}
	req->reply.rc = 0;
}

void do_rmv(datastore_t* datastore, request_t* req)
{
	req->reply.rc = datastore_remove(datastore, req->name);
}

void do_md5(datastore_t* datastore, request_t* req)
{
	char digest_str[MD5_DIGEST_STR_LENGTH];
	md5_str(req->value, strlen(req->value), digest_str);
	req->reply.rc = datastore_set(datastore, req->name, digest_str);
}

void do_sha1(datastore_t* datastore, request_t* req)
{
	char digest_str[SHA1_DIGEST_STR_LENGTH];
	SHA1_str(req->value, strlen(req->value), digest_str);
	req->reply.rc = datastore_set(datastore, req->name, digest_str);
}