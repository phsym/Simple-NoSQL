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
 * config.h
 *
 *  Created on: 12 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "utils.h"

#define CONFIG_BUFF_SIZE 1024

typedef struct{
	char* file;

	DBG_LVL debug_lvl;
	int storage_size;
	int index_len;
	short bind_port;
	unsigned int bind_address;
}config_t;

void config_load(config_t* config, char* file);

#endif /* CONFIG_H_ */
