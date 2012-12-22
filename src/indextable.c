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
 * indextable.c
 *
 *  Created on: 30 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include "indextable.h"

#include <string.h>
#include <stdlib.h>

index_table_t* index_table_create(int capacity)
{
	index_table_t* table = malloc(sizeof(index_table_t));
	table->capacity = capacity;
	table->lists = malloc(sizeof(linked_list_t)*capacity);

	int i;
	for(i = 0; i < capacity; i++)
		linked_list_init(table->lists + i);

	return table;
}

void index_table_put(index_table_t* table, char* key, int ptr)
{
	int index = hash(key, strlen(key)) % table->capacity;
	linked_list_t *list = (table->lists + index);
	void* iterator;
	index_t *ind = linked_list_iterate(list, &iterator);
	while(ind != NULL)
	{
		if(strcmp(ind->key, key) == 0)
		{
//			ind->ptr = ptr;
			return;
		}
		ind = linked_list_iterate(NULL, &iterator);
	}

	ind = malloc(sizeof(index_t));
	ind->key = key;
	ind->ptr = ptr;
	linked_list_append(list, ind);
}

int index_table_get(index_table_t* table, char* key)
{
	int index = hash(key, strlen(key)) % table->capacity;
	linked_list_t *list = (table->lists + index);
	void* iterator;
	index_t *ind = linked_list_iterate(list, &iterator);
	while(ind != NULL)
	{
		if(ind->key != NULL && strcmp(ind->key, key) == 0)
			return ind->ptr;
		ind = linked_list_iterate(NULL, &iterator);
	}
	return -1;
}

void index_table_remove(index_table_t* table, char* key)
{
	int index = hash(key, strlen(key)) % table->capacity;
	linked_list_t *list = (table->lists + index);
	void* iterator;
	index_t *ind = linked_list_iterate(list, &iterator);
	int i = 0;
	while(ind != NULL)
	{
		if(strcmp(ind->key, key) == 0)
		{
			linked_list_remove(list, i, 1);
			return;
		}
		ind = linked_list_iterate(NULL, &iterator);
		i++;
	}
}

void index_table_clean(index_table_t* table)
{
	int i;
	for(i = 0; i < table->capacity; i++)
		linked_list_clean(table->lists + i, 1);
}

void index_table_destroy(index_table_t* table)
{
	int i;
	for(i = 0; i < table->capacity; i++)
		linked_list_clean(table->lists + i, 1);
	free(table->lists);
	free(table);
}

int index_table_count_keys(index_table_t* table)
{
	int count = 0;
	int i;
	for(i = 0; i < table->capacity; i++)
	{
		linked_list_t *list = (table->lists + i);
		if(list != NULL)
		{
			void * iterator;
			index_t *ind = linked_list_iterate(list, &iterator);
			while(ind != NULL)
			{
				count++;
				ind = linked_list_iterate(NULL, &iterator);
			}
		}
	}
	return count;
}

void index_table_list_keys(index_table_t* table, char** keys, int len)
{
	int i;
	int c = 0;
	for(i = 0; i < table->capacity; i++)
	{
		linked_list_t *list = (table->lists + i);
		if(list != NULL)
		{
			void * iterator;
			index_t *ind = linked_list_iterate(list, &iterator);
			while(ind != NULL)
			{
				if(c >= len)
					return;
				keys[c++] = ind->key;
				ind = linked_list_iterate(NULL, &iterator);
			}
		}
	}
}

//void index_table_rehash(index_table_t* table, int new_capacity)
//{
//	//TODO : implement it
//}

unsigned int hash(char* str, int str_len) {
	unsigned int h = 0;
	int i;
	for (i = 0; i < str_len; i++) {
		h = 31 * h + str[i];
	}
	return h;
}
