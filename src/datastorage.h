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
#include "indextable.h"

typedef struct {
	char name[32];
	char value[32];
}data_t;

typedef struct {
	index_table_t *index_table;
	table_t *data_table;
	mutex_t mutex;
}datastore_t;

datastore_t* datastore_create(int storage_size, int index_length);

char* datastore_lookup(datastore_t* datastore, char* key);

int datastore_put(datastore_t* datastore, char* key, char* value);

int datastore_remove(datastore_t* datastore, char* key);

int datastore_count_keys(datastore_t* datastore);

void datastore_list_keys(datastore_t* datastore, char **keys, int len);

void datastore_destroy(datastore_t* datastore);

#endif /* DATASTORAGE_H_ */
