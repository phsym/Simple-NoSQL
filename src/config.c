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
 * config.c
 *
 *  Created on: 12 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MINGW32__
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif


#include "utils.h"

config_param_t conf_p[] = {
	{"port", &config_port},
	{"address", &config_address},
	{"debug_level", &config_debug_lvl},
	{"storage_size", &config_storage_sz},
	{"index_length", &config_index_len},
	{"auth", &config_auth}
};

void config_port(config_t* config, char* value)
{
	config->bind_port = atoi(value)&0x0000ffff;
}

void config_address(config_t* config, char* value)
{
	config->bind_address = inet_addr(value);
}

void config_debug_lvl(config_t* config, char* value)
{
	int i;
	DBG_LVL lvl = LVL_INFO; //Default value
	for(i = 0; i < MAX_DEBUG_LEVEL; i++)
	{
		if(!strcmp(value, DBG_LVL_STR[i]))
		{
			lvl = i;
			break;
		}
	}
	config->debug_lvl = lvl;
	DEBUG_LEVEL = lvl;
}

void config_storage_sz(config_t* config, char* value)
{
	config->storage_size = atoi(value);
}

void config_index_len(config_t* config, char* value)
{
	config->index_len = atoi(value);
}

void config_auth(config_t* config, char* value)
{
	if(!strcmp(value, "yes"))
		config->auth = true;
}

void config_load(config_t* config, char* file)
{
	config->file = file;

	//Default settings
	config->bind_address = 0;
	config->bind_port = 1234;
	config->debug_lvl = DEBUG_LEVEL;
	config->index_len = 1024*1024;
	config->storage_size = 1024*1024;
	config->auth = false;

	FILE* fd = fopen(file, "r");
	if(fd == NULL)
	{
		_log(LVL_WARNING, "Config file %s not found, using defaults\n", file);
		return;
	}
	_log(LVL_DEBUG, "Config file opened\n");
	char str[CONFIG_BUFF_SIZE];
	while(!feof(fd))
	{
		if(fgets(str, CONFIG_BUFF_SIZE, fd) == NULL)
			break;

		//Process line
		
		char* c = strchr(str, '\n');
		if(c != NULL)
			c[0] = '\0';
		c = strchr(str, '\r');
		if(c != NULL)
			c[0] = '\0';

		if(str[0] == '#' || strlen(str) <= 0)
			continue;

		char* it[CONFIG_BUFF_SIZE];

		char* param = strtok_r(str, "=", it);
		char* value = strtok_r(NULL, "=", it);

		_log(LVL_DEBUG, "%s = %s\n", param, value);
		
		int num_param = sizeof(conf_p)/sizeof(config_param_t);
		int i;
		bool found = false;
		for (i = 0; i < num_param; i++)
		{
			if(!strcmp(param, conf_p[i].name))
			{
				conf_p[i].conf_hdl(config, value);
				found = true;
			}
		}
		
		if(!found)
			_log(LVL_WARNING, "Unknown config parameter : %s = %s\n", param, value);
	}
	fclose(fd);
}
