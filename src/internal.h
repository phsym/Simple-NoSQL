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
 * internal.h
 *
 *  Created on: 28 janv. 2013
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef INTERNAL_H_
#define INTERNAL_H_

#include <stdint.h>

#include "datastorage.h"
#include "containers.h"

#define INT_DB_LIST "DATABASES"
#define INT_DEAFUALT_DB "DEFAULTDB"
#define INT_USER_HASH "DB_ADM.USER.AUTH_HASH"

#define PREFIX_DB "DB."
#define SUFFIX_DB_STORAGE_SIZE ".STORAGE_SIZE"
#define SUFFIX_DB_INDEX_SIZE ".INDEX_SIZE"

#define CAT4(out, a, b, c)	{out[0] = '\0'; \
							strcat(out, a); \
							strcat(out, b); \
							strcat(out, c);}

uint64_t intern_get_storage_size(datastore_t* int_db, char* dbname);
uint64_t intern_get_storage_index_len(datastore_t* int_db, char* dbname);

int intern_set_storage_size(datastore_t* int_db, char* dbname, char* size);
int intern_set_storage_index_len(datastore_t* int_db, char* dbname, char* len);

int intern_set_default_db(datastore_t* int_db, hashtable_t* table, char* dbname);
datastore_t* intern_get_default_db(datastore_t* int_db, hashtable_t* table);

void intern_load_storages(datastore_t* int_db, hashtable_t* table);
int intern_create_new_db(datastore_t* int_db, hashtable_t* table, char* dbname, char* store_size, char* index_len);


#endif /* INTERNAL_H_ */
