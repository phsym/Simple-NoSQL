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
 * datastorage.h
 *
 *  Created on: 3 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef DATASTORAGE_H_
#define DATASTORAGE_H_

#include "concurrency.h"
#include "table.h"
#include "containers.h"
#include "utils.h"

#define BLOCK_SIZE 32

typedef struct {
	int key_size;
	int value_size;
	char data[];
}data_t;

#define data_dt(ks, vs) struct { \
							int key_size; \
							int value_size; \
							char key[ks+1]; \
							char value[vs+1]; \
						}

typedef struct {
	char* name;
	hashtable_t *index_table;
	table_t *data_table;
	rw_lock_t lock;
}datastore_t;

datastore_t* datastore_create(char* name, uint64_t storage_size, uint64_t index_length);

char* datastore_lookup(datastore_t* datastore, char* key);

int datastore_put(datastore_t* datastore, char* key, char* value);

int datastore_set(datastore_t* datastore, char* key, char* value);

bool datastore_exists(datastore_t* datastore, char* key);

int datastore_remove(datastore_t* datastore, char* key);

uint64_t datastore_keys_number(datastore_t* datastore);

uint64_t datastore_count_keys(datastore_t* datastore);

void datastore_list_keys(datastore_t* datastore, char **keys, uint64_t len);

void datastore_clear(datastore_t* datastore);

void datastore_destroy(datastore_t* datastore);

#endif /* DATASTORAGE_H_ */
