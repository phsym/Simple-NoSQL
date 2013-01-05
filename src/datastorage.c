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


datastore_t* datastore_create(int storage_size, int index_length)
{
	char* storagefile = "./datastorage.dat";
	datastore_t* store = malloc(sizeof(datastore_t));
	store->index_table = hashtable_create(index_length);
	store->data_table = table_map_load(storagefile);
	if(store->data_table == NULL)
	{
		_log(LVL_INFO, "Creating new data table ...\n");
		store->data_table = table_map_create(storagefile, sizeof(data_t), (storage_size - sizeof(table_t))/(sizeof(data_t)+sizeof(table_elem_t)));
	}
	else
	{
		int i;
		int p;
		data_t* tmp;
		for(i = 0; i < store->data_table->capacity; i++)
		{
			p = (100 * i)/store->data_table->capacity;
			if(i%(store->data_table->capacity/100) == 0)
			{
				_log(LVL_INFO, "Rebuilding index table : %d%%\r", p);
				fflush(stdout);
			}
			tmp = table_get_ref(store->data_table, i);
			if(tmp != NULL)
			{
				_log(LVL_TRACE, "\nfound %s at index %d\n", tmp->name, i);
				hashtable_put(store->index_table, tmp->name, i);
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
	int index = hashtable_get(datastore->index_table, key);
	if(index >= 0)
	{
		data_t* data = table_get_ref(datastore->data_table, index);
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
	rw_lock_write_lock(&datastore->lock);
	int index = hashtable_get(datastore->index_table, key);
	if(index < 0)
	{
		data_t data;
		strncpy(data.name, key, MAX_KEY_SIZE+1);
		strncpy(data.value, value, MAX_VALUE_SIZE+1);
		index = table_put(datastore->data_table, &data);
		hashtable_put(datastore->index_table, key, index);
		rw_lock_write_unlock(&datastore->lock);
		return 0;
	}
	rw_lock_write_unlock(&datastore->lock);
	return -1;
}

int datastore_set(datastore_t* datastore, char* key, char* value)
{
	rw_lock_write_lock(&datastore->lock);
	int index = hashtable_get(datastore->index_table, key);
	if (index < 0) {
		data_t data;
		strncpy(data.name, key, MAX_KEY_SIZE+1);
		strncpy(data.value, value, MAX_VALUE_SIZE+1);
		index = table_put(datastore->data_table, &data);
		hashtable_put(datastore->index_table, key, index);
	}
	else
	{
		data_t *data = table_get_ref(datastore->data_table, index);
		strncpy(data->value, value, MAX_VALUE_SIZE+1);
	}
	rw_lock_write_unlock(&datastore->lock);
	return 0;
}

int datastore_remove(datastore_t* datastore, char* key)
{
	rw_lock_write_lock(&datastore->lock);
	int index = hashtable_get(datastore->index_table, key);
	if(index >= 0)
	{
		table_remove(datastore->data_table, index);
		hashtable_remove(datastore->index_table, key);
		rw_lock_write_unlock(&datastore->lock);
		return 0;
	}
	rw_lock_write_unlock(&datastore->lock);
	return -1;
}

int datastore_keys_number(datastore_t* datastore)
{
	rw_lock_read_lock(&datastore->lock);
	int n = hashtable_keys_number(datastore->index_table);
	rw_lock_read_unlock(&datastore->lock);
	return n;
}

int datastore_count_keys(datastore_t* datastore)
{
	rw_lock_read_lock(&datastore->lock);
	int i = hashtable_count_keys(datastore->index_table);
	rw_lock_read_unlock(&datastore->lock);
	return i;
}

void datastore_list_keys(datastore_t* datastore, char **keys, int len)
{
	rw_lock_read_lock(&datastore->lock);
	hashtable_list_keys(datastore->index_table, keys, len);
	rw_lock_read_unlock(&datastore->lock);
}

void datastore_destroy(datastore_t* datastore)
{
	rw_lock_write_lock(&datastore->lock);
	hashtable_destroy(datastore->index_table);
	destroy_map_table(datastore->data_table);
	rw_lock_destroy(&datastore->lock);
	free(datastore);
}
