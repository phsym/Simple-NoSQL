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
 * datastorage.c
 *
 *  Created on: 3 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include "datastorage.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "containers.h"

datastore_t* datastore_create(char* name, uint64_t storage_size, uint64_t index_length)
{
	char storagefile[strlen(name) + 8];
	storagefile[0] = '\0';
	strcat(storagefile, "db_");
	strcat(storagefile, name);
	strcat(storagefile, ".dat");
	datastore_t* store = malloc(sizeof(datastore_t));
	store->name = malloc(strlen(name) + 1);
	strcpy(store->name, name);
	store->index_table = ht_create(index_length);
	store->data_table = table_map_load(storagefile);
	if(store->data_table == NULL)
	{
		_log(LVL_INFO, "Creating new data table ...\n");
		_log(LVL_DEBUG,"storage size = %llu\n", storage_size);
		_log(LVL_DEBUG,"Data size = %llu\n", sizeof(data_t));
		_log(LVL_DEBUG,"capacity = %llu\n", (storage_size - sizeof(table_t))/(sizeof(data_t)+sizeof(table_elem_t)));
		store->data_table = table_map_create(storagefile, BLOCK_SIZE, (storage_size - sizeof(table_t))/(BLOCK_SIZE+sizeof(table_elem_t)));
		if(store->data_table == NULL)
		{
			_log(LVL_ERROR, "Could not create table %s\n", name);
			free(store);
			return NULL;
		}
	}
	else
	{
		uint64_t i;
		uint64_t p;
		table_elem_t* tmp;
		data_t data;
		for(i = 0; i < store->data_table->capacity; i++)
		{
			p = (100 * i)/store->data_table->capacity;
			if(store->data_table->capacity >= 100 && i%(store->data_table->capacity/100) == 0)
			{
				_log(LVL_INFO, "Rebuilding index table : %d%%\r", p);
				fflush(stdout);
			}
			tmp = table_get_block(store->data_table, i);
			if(tmp != NULL && (tmp->flag & FLAG_HEAD))
			{
				while((tmp->flag & FLAG_END) == 0)
					tmp = table_get_block(store->data_table, tmp->ind);
				table_get_copy(store->data_table, i, &data, sizeof(data_t));
				data_dt(data.key_size, data.value_size) dat;
				dat.key[0] = '\0';
				table_get_copy(store->data_table, i, &dat, sizeof(dat));
				_log(LVL_TRACE, "found %s at index %d\n", dat.key, i);
				ht_put(store->index_table, dat.key, &tmp->ind);
			}
		}
		p = (100 * i)/store->data_table->capacity;
		_log(LVL_INFO, "Rebuilding index table : %d%%\r\n", p);
	}
	rw_lock_init(&store->lock);
	return store;
}

char* datastore_lookup(datastore_t* datastore, char* key)
{
	rw_lock_read_lock(&datastore->lock);
	char* value = NULL;
	uint64_t* index;
	index = ht_get(datastore->index_table, key);
	if(index != NULL)
	{
		data_t data;
		if(table_get_copy(datastore->data_table, *index, &data, sizeof(data_t)) > 0)
		{
			data_dt(data.key_size, data.value_size) dat;
			table_get_copy(datastore->data_table, *index, &dat, sizeof(dat));
			value = malloc(dat.value_size + 1);
			if(value == NULL)
				return NULL;
			strncpy(value, dat.value, dat.value_size+1);
		}
	}
	rw_lock_read_unlock(&datastore->lock);
	return value;
}

int datastore_put(datastore_t* datastore, char* key, char* value)
{
	rw_lock_write_lock(&datastore->lock);
	uint64_t* index;
	index = ht_get(datastore->index_table, key);
	if(index == NULL)
	{
		int ks = strlen(key);
		int vs = strlen(value);
		data_dt(ks, vs) data;
		data.key_size = ks;
		data.value_size = vs;
		strncpy(data.key, key, ks+1);
		strncpy(data.value, value, vs+1);
		index = table_put(datastore->data_table, &data, sizeof(data));
		if(ht_put(datastore->index_table, key, index) == HT_ERROR)
		{
			table_remove(datastore->data_table, *index);
			rw_lock_write_unlock(&datastore->lock);
			return -1;
		}
		rw_lock_write_unlock(&datastore->lock);
		return 0;
	}
	rw_lock_write_unlock(&datastore->lock);
	return -1;
}

int datastore_set(datastore_t* datastore, char* key, char* value)
{
	rw_lock_write_lock(&datastore->lock);
	uint64_t* index;
	int ks = strlen(key);
	int vs = strlen(value);
	data_dt(ks, vs) data;
	data.key_size = ks;
	data.value_size = vs;
	strncpy(data.key, key, ks+1);
	strncpy(data.value, value, vs+1);
	index = ht_get(datastore->index_table, key);
	if (index == NULL)
	{
		index = table_put(datastore->data_table, &data, sizeof(data));
		if(ht_put(datastore->index_table, key, index) == HT_ERROR)
		{
			table_remove(datastore->data_table, *index);
			rw_lock_write_unlock(&datastore->lock);
			return -1;
		}
	}
	else
	{
		table_remove(datastore->data_table, *index);
		index = table_put(datastore->data_table, &data, sizeof(data));
		if(ht_put(datastore->index_table, key, index) == HT_ERROR)
		{
			table_remove(datastore->data_table, *index);
			rw_lock_write_unlock(&datastore->lock);
			return -1;
		}

	}
	rw_lock_write_unlock(&datastore->lock);
	return 0;
}

bool datastore_exists(datastore_t* datastore, char* key)
{
	rw_lock_read_lock(&datastore->lock);
	bool ret = false;
	if(ht_get(datastore->index_table, key) != NULL)
		ret = true;
	rw_lock_read_unlock(&datastore->lock);
	return ret;
}

int datastore_remove(datastore_t* datastore, char* key)
{
	rw_lock_write_lock(&datastore->lock);
	uint64_t* index;
	index = ht_get(datastore->index_table, key);
	if(index != NULL)
	{
		table_remove(datastore->data_table, *index);
		ht_remove(datastore->index_table, key);
		rw_lock_write_unlock(&datastore->lock);
		return 0;
	}
	rw_lock_write_unlock(&datastore->lock);
	return -1;
}

uint64_t datastore_keys_number(datastore_t* datastore)
{
	rw_lock_read_lock(&datastore->lock);
	uint64_t n = datastore->index_table->e_num;
	rw_lock_read_unlock(&datastore->lock);
	return n;
}

uint64_t datastore_count_keys(datastore_t* datastore)
{
	rw_lock_read_lock(&datastore->lock);
//	uint64_t i = hashtable_count_keys(datastore->index_table);
	uint64_t i = datastore->index_table->e_num;
	rw_lock_read_unlock(&datastore->lock);
	return i;
}

void datastore_list_keys(datastore_t* datastore, char **keys, uint64_t len)
{
	rw_lock_read_lock(&datastore->lock);
	ht_list_keys(datastore->index_table, keys, len);
	rw_lock_read_unlock(&datastore->lock);
}

void datastore_clear(datastore_t* datastore)
{
	rw_lock_read_lock(&datastore->lock);
	hash_elem_it it = HT_ITERATOR(datastore->index_table);
	hash_elem_t* e;
	uint64_t* ind;
	while((e = ht_iterate(&it)) != NULL)
	{
		ind = e->data;
		table_remove(datastore->data_table, *ind);
		ht_remove(datastore->index_table, e->key);
	}
	rw_lock_read_unlock(&datastore->lock);
}

void datastore_destroy(datastore_t* datastore)
{
	rw_lock_write_lock(&datastore->lock);
	ht_clear(datastore->index_table, 0);
	ht_destroy(datastore->index_table);
	destroy_map_table(datastore->data_table);
	rw_lock_destroy(&datastore->lock);
	free(datastore->name);
	free(datastore);
}
