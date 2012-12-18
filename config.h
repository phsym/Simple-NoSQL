/*
 * config.h
 *
 *  Created on: 12 août 2012
 *      Author: phsymo10
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
