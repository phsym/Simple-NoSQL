/*
 * protocol.c
 *
 *  Created on: 8 août 2012
 *      Author: phsymo10
 */


#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "utils.h"

unsigned int last_id = 0;

int decode_request(request_t* request, char* req, int len)
{
	memset(request, 0, sizeof(request));

	request->reply.replied = 0;
	request->id = last_id ++;

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
	else if(strcmp(op, "put") == 0)
	{
		request->op = OP_PUT;
		char* name = strtok_r(NULL, " ", str);
		request->name = name;
		char* val = strtok_r(NULL, " ", str);
		request->value = val;
		if(name == NULL || val == NULL)
			return -1;
	}
	else if(strcmp(op, "get") == 0)
	{
		request->op = OP_GET;
		char* name = strtok_r(NULL, " ", str);
		request->name = name;
		if(name == NULL)
			return -1;
	}
	else if(strcmp(op, "list") == 0)
	{
		request->op = OP_LIST;
	}
	else if(strcmp(op, "rmv") == 0)
	{
		request->op = OP_RMV;
		char* name = strtok_r(NULL, " ", str);
		request->name = name;
		if(name == NULL)
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
			default:
				break;
		}
	}
	else
	{
		strcat(buff, "NOK\r\n");
		strcat(buff,  req->reply.message);
		strcat(buff, "\r\n");
	}
}
