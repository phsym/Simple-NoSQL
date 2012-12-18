/*
 * datastorage.c
 *
 *  Created on: 3 août 2012
 *      Author: phsymo10
 */

#include "datastorage.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

datastore_t* datastore_create(int storage_size, int index_length)
{
	char* storagefile = "./tmp/datastorage.dat";
	datastore_t* store = malloc(sizeof(datastore_t));
	store->index_table = index_table_create(index_length);
	store->data_table = table_map_load(storagefile);
	if(store->data_table == NULL)
	{
		_log(LVL_INFO, "Creating new data table ...\n");
		store->data_table = table_map_create(storagefile, sizeof(data_t), storage_size);
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
				index_table_put(store->index_table, tmp->name, i);
			}
		}
		p = (100 * i)/store->data_table->capacity;
		_log(LVL_INFO, "Rebuilding index table : %d%%\r\n", p);
	}
	semaphore_init(&store->semaphore, 1, 10);
	return store;
}

char* datastore_lookup(datastore_t* datastore, char* key)
{
	semaphore_wait(&datastore->semaphore);
	char* value = NULL;
	int index = index_table_get(datastore->index_table, key);
	if(index >= 0)
	{
		data_t* data = table_get_ref(datastore->data_table, index);
		if(data != NULL)
		{
			value = data->value;
		}
	}
	semaphore_post(&datastore->semaphore);
	return value;
}

int datastore_put(datastore_t* datastore, char* key, char* value)
{
	semaphore_wait(&datastore->semaphore);
	int index = index_table_get(datastore->index_table, key);
	if(index < 0)
	{
		data_t data;
		strncpy(data.name, key, 32);
		strncpy(data.value, value, 32);
		index = table_put(datastore->data_table, &data);
		index_table_put(datastore->index_table, key, index);
		semaphore_post(&datastore->semaphore);
		return 0;
	}
	semaphore_post(&datastore->semaphore);
	return -1;
}

int datastore_remove(datastore_t* datastore, char* key)
{
	semaphore_wait(&datastore->semaphore);
	int index = index_table_get(datastore->index_table, key);
	if(index >= 0)
	{
		table_remove(datastore->data_table, index);
		index_table_remove(datastore->index_table, key);
		semaphore_post(&datastore->semaphore);
		return 0;
	}
	semaphore_post(&datastore->semaphore);
	return -1;
}

int datastore_count_keys(datastore_t* datastore)
{
	semaphore_wait(&datastore->semaphore);
	int i = index_table_count_keys(datastore->index_table);
	semaphore_post(&datastore->semaphore);
	return i;
}

void datastore_list_keys(datastore_t* datastore, char **keys, int len)
{
	semaphore_wait(&datastore->semaphore);
	index_table_list_keys(datastore->index_table, keys, len);
	semaphore_post(&datastore->semaphore);
}

void datastore_destroy(datastore_t* datastore)
{
	semaphore_wait(&datastore->semaphore);
	index_table_destroy(datastore->index_table);
	destroy_map_table(datastore->data_table);
	semaphore_destroy(&datastore->semaphore);
	free(datastore);
}
