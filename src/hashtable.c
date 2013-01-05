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
 * hashtable.c
 *
 *  Created on: 30 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include "hashtable.h"

#include <string.h>
#include <stdlib.h>

hashtable_t* hashtable_create(int capacity)
{
	hashtable_t* table = malloc(sizeof(hashtable_t));
	table->capacity = capacity;
	table->keys_num = 0;
	table->lists = malloc(sizeof(linked_list_t)*capacity);

	int i;
	for(i = 0; i < capacity; i++)
		linked_list_init(table->lists + i);

	return table;
}

void hashtable_put(hashtable_t* table, char* key, uintptr_t ptr)
{
	int index = hash(key, strlen(key)) % table->capacity;
	linked_list_t *list = (table->lists + index);
	void* iterator;
	hashtable_elem_t *ind = linked_list_iterate(list, &iterator);
	while(ind != NULL)
	{
		if(strcmp(ind->key, key) == 0)
		{
//			ind->ptr = ptr;
			return;
		}
		ind = linked_list_iterate(NULL, &iterator);
	}

	ind = malloc(sizeof(hashtable_elem_t)+strlen(key)+1);
	strcpy(ind->key, key);
	ind->ptr = ptr;
	linked_list_append(list, ind);
	table->keys_num ++;
}

uintptr_t hashtable_get(hashtable_t* table, char* key)
{
	int index = hash(key, strlen(key)) % table->capacity;
	linked_list_t *list = (table->lists + index);
	void* iterator;
	hashtable_elem_t *ind = linked_list_iterate(list, &iterator);
	while(ind != NULL)
	{
		if(ind->key != NULL && strcmp(ind->key, key) == 0)
			return ind->ptr;
		ind = linked_list_iterate(NULL, &iterator);
	}
	return -1;
}

void hashtable_remove(hashtable_t* table, char* key)
{
	int index = hash(key, strlen(key)) % table->capacity;
	linked_list_t *list = (table->lists + index);
	void* iterator;
	hashtable_elem_t *ind = linked_list_iterate(list, &iterator);
	int i = 0;
	while(ind != NULL)
	{
		if(strcmp(ind->key, key) == 0)
		{
			linked_list_remove(list, i, 1);
			table->keys_num --;
			return;
		}
		ind = linked_list_iterate(NULL, &iterator);
		i++;
	}
}

void hashtable_clean(hashtable_t* table)
{
	int i;
	for(i = 0; i < table->capacity; i++)
		linked_list_clean(table->lists + i, 1);
	table->keys_num = 0;
}

void hashtable_destroy(hashtable_t* table)
{
	hashtable_clean(table);
	free(table->lists);
	free(table);
}

int hashtable_keys_number(hashtable_t* table)
{
	return table->keys_num;
}

int hashtable_count_keys(hashtable_t* table)
{
	int count = 0;
	int i;
	for(i = 0; i < table->capacity; i++)
	{
		linked_list_t *list = (table->lists + i);
		if(list != NULL)
		{
			void * iterator;
			hashtable_elem_t *ind = linked_list_iterate(list, &iterator);
			while(ind != NULL)
			{
				count++;
				ind = linked_list_iterate(NULL, &iterator);
			}
		}
	}
	return count;
}

void hashtable_list_keys(hashtable_t* table, char** keys, int len)
{
	int i;
	int c = 0;
	for(i = 0; i < table->capacity; i++)
	{
		linked_list_t *list = (table->lists + i);
		if(list != NULL)
		{
			void * iterator;
			hashtable_elem_t *ind = linked_list_iterate(list, &iterator);
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

//void hashtable_rehash(index_table_t* table, int new_capacity)
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