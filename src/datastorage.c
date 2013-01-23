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
#include "utils.h"
#include "containers.h"

typedef union {
	void* v;
	int i;
} pi_u;

datastore_t* datastore_create(int storage_size, int index_length)
{
	char* storagefile = "./datastorage.dat";
	datastore_t* store = malloc(sizeof(datastore_t));
	store->index_table = ht_create(index_length);
	store->data_table = table_map_load(storagefile);
	if(store->data_table == NULL)
	{
		_log(LVL_INFO, "Creating new data table ...\n");
		store->data_table = table_map_create(storagefile, sizeof(data_t), (storage_size - sizeof(table_t))/(sizeof(data_t)+sizeof(table_elem_t)));
	}
	else
	{
		pi_u pi;
		int p;
		data_t* tmp;
		for(pi.i = 0; pi.i < store->data_table->capacity; pi.i++)
		{
			p = (100 * pi.i)/store->data_table->capacity;
			if(pi.i%(store->data_table->capacity/100) == 0)
			{
				_log(LVL_INFO, "Rebuilding index table : %d%%\r", p);
				fflush(stdout);
			}
			tmp = table_get_ref(store->data_table, pi.i);
			if(tmp != NULL)
			{
				_log(LVL_TRACE, "found %s at index %d\n", tmp->name, pi.i);
				ht_put(store->index_table, tmp->name, pi.v);
			}
		}
		p = (100 * pi.i)/store->data_table->capacity;
		_log(LVL_INFO, "Rebuilding index table : %d%%\r\n", p);
	}
	rw_lock_init(&store->lock);
	return store;
}

char* datastore_lookup(datastore_t* datastore, char* key)
{
	CHECK_KEY_SIZE(key);
	rw_lock_read_lock(&datastore->lock);
	char* value = NULL;
	pi_u index;
	index.v = ht_get(datastore->index_table, key);
	if(index.v != NULL)
	{
		data_t* data = table_get_ref(datastore->data_table, index.i);
		if(data != NULL)
		{
			value = data->value;
		}
	}
	rw_lock_read_unlock(&datastore->lock);
	return value;
}

int datastore_put(datastore_t* datastore, char* key, char* value)
{
	CHECK_KEY_SIZE(key);
	CHECK_VALUE_SIZE(value);
	rw_lock_write_lock(&datastore->lock);
	pi_u index;
	index.v = ht_get(datastore->index_table, key);
	if(index.v == NULL)
	{
		data_t data;
		strncpy(data.name, key, MAX_KEY_SIZE+1);
		strncpy(data.value, value, MAX_VALUE_SIZE+1);
		index.i = table_put(datastore->data_table, &data);
		if(ht_put(datastore->index_table, key, index.v) == HT_ERROR)
		{
			table_remove(datastore->data_table, index.i);
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
	CHECK_KEY_SIZE(key);
	CHECK_VALUE_SIZE(value);
	rw_lock_write_lock(&datastore->lock);
	pi_u index;
	index.v = ht_get(datastore->index_table, key);
	if (index.v == NULL) {
		data_t data;
		strncpy(data.name, key, MAX_KEY_SIZE+1);
		strncpy(data.value, value, MAX_VALUE_SIZE+1);
		index.i = table_put(datastore->data_table, &data);
		if(ht_put(datastore->index_table, key, index.v) == HT_ERROR)
		{
			table_remove(datastore->data_table, index.i);
			rw_lock_write_unlock(&datastore->lock);
			return -1;
		}
	}
	else
	{
		data_t *data = table_get_ref(datastore->data_table, index.i);
		strncpy(data->value, value, MAX_VALUE_SIZE+1);
	}
	rw_lock_write_unlock(&datastore->lock);
	return 0;
}

int datastore_remove(datastore_t* datastore, char* key)
{
	CHECK_KEY_SIZE(key);
	rw_lock_write_lock(&datastore->lock);
	pi_u index;
	index.v = ht_get(datastore->index_table, key);
	if(index.v != NULL)
	{
		table_remove(datastore->data_table, index.i);
		ht_remove(datastore->index_table, key);
		rw_lock_write_unlock(&datastore->lock);
		return 0;
	}
	rw_lock_write_unlock(&datastore->lock);
	return -1;
}

int datastore_keys_number(datastore_t* datastore)
{
	rw_lock_read_lock(&datastore->lock);
	int n = datastore->index_table->e_num;
	rw_lock_read_unlock(&datastore->lock);
	return n;
}

int datastore_count_keys(datastore_t* datastore)
{
	rw_lock_read_lock(&datastore->lock);
//	int i = hashtable_count_keys(datastore->index_table);
	int i = datastore->index_table->e_num;
	rw_lock_read_unlock(&datastore->lock);
	return i;
}

void datastore_list_keys(datastore_t* datastore, char **keys, int len)
{
	rw_lock_read_lock(&datastore->lock);
	ht_list_keys(datastore->index_table, keys, len);
	rw_lock_read_unlock(&datastore->lock);
}

void datastore_destroy(datastore_t* datastore)
{
	rw_lock_write_lock(&datastore->lock);
	ht_destroy(datastore->index_table);
	destroy_map_table(datastore->data_table);
	rw_lock_destroy(&datastore->lock);
	free(datastore);
}
