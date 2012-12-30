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
		request->name = strtok_r(NULL, " ", str);
		request->value = strtok_r(NULL, " ", str);
		if(request->name == NULL || request->value == NULL)
			return -1;
	}
	else if(strcmp(op, "set") == 0)
	{
		request->op = OP_SET;
		request->name = strtok_r(NULL, " ", str);
		request->value = strtok_r(NULL, " ", str);
		if(request->name == NULL || request->value == NULL)
			return -1;

	}
	else if(strcmp(op, "get") == 0)
	{
		request->op = OP_GET;
		request->name = strtok_r(NULL, " ", str);
		if(request->name == NULL)
			return -1;
	}
	else if(strcmp(op, "list") == 0)
	{
		request->op = OP_LIST;
	}
	else if(strcmp(op, "rmv") == 0)
	{
		request->op = OP_RMV;
		request->name = strtok_r(NULL, " ", str);
		if(request->name == NULL)
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
			case OP_LIST:
				strcat(buff, req->reply.message);
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
