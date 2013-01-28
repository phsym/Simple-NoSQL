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
 * containers.c
 *
 *  Created on: 22 janv. 2013
 *      Author: Pierre-Henri Symoneaux
 */

#include <stdlib.h>
#include <string.h>

#include "containers.h"

char err_ptr;
void* HT_ERROR = &err_ptr;

/*****************************************
 *            HASHTABLE                  *
 *****************************************/

/* 	Internal funcion to calculate hash for keys.
	It's based on the DJB algorithm from Daniel J. Bernstein.
	The key must be ended by '\0' character.*/
static unsigned int ht_calc_hash(char* key)
{
	unsigned int h = 5381;
	while(*(key++))
		h = ((h << 5) + h) + (*key);
	return h;
}

/* 	Create a hashtable with capacity 'capacity'
	and return a pointer to it*/
hashtable_t* ht_create(unsigned int capacity)
{
	hashtable_t* hasht = malloc(sizeof(hashtable_t));
	if(!hasht)
		return NULL;
	if((hasht->table = malloc(capacity*sizeof(hash_elem_t*))) == NULL)
	{
		free(hasht->table);
		return NULL;
	}
	hasht->capacity = capacity;
	hasht->e_num = 0;
	unsigned int i;
	for(i = 0; i < capacity; i++)
		hasht->table[i] = NULL;
	return hasht;
}

/* 	Store data in the hashtable. If data with the same key are already stored,
	they are overwritten, and return by the function. Else it return NULL.
	Return HT_ERROR if there are memory alloc error*/
void* ht_put(hashtable_t* hasht, char* key, void* data)
{
	if(data == NULL)
		return NULL;
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hash_elem_t* e = hasht->table[h];

	while(e != NULL)
	{
		if(!strcmp(e->key, key))
		{
			void* ret = e->data;
			e->data = data;
			return ret;
		}
		e = e->next;
	}

	// Getting here means the key doesn't already exist

	if((e = malloc(sizeof(hash_elem_t)+strlen(key)+1)) == NULL)
		return HT_ERROR;
	strcpy(e->key, key);
	e->data = data;

	// Add the element at the beginning of the linked list
	e->next = hasht->table[h];
	hasht->table[h] = e;
	hasht->e_num ++;

	return NULL;
}

/* Retrieve data from the hashtable */
void* ht_get(hashtable_t* hasht, char* key)
{
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hash_elem_t* e = hasht->table[h];
	while(e != NULL)
	{
		if(!strcmp(e->key, key))
			return e->data;
		e = e->next;
	}
	return NULL;
}

/* 	Remove data from the hashtable. Return the data removed from the table
	so that we can free memory if needed */
void* ht_remove(hashtable_t* hasht, char* key)
{
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hash_elem_t* e = hasht->table[h];
	hash_elem_t* prev = NULL;
	while(e != NULL)
	{
		if(!strcmp(e->key, key))
		{
			void* ret = e->data;
			if(prev != NULL)
				prev->next = e->next;
			else
				hasht->table[h] = e->next;
			free(e);
			e = NULL;
			hasht->e_num --;
			return ret;
		}
		prev = e;
		e = e->next;
	}
	return NULL;
}

/* List keys. k should have length equals or greater than the number of keys */
void ht_list_keys(hashtable_t* hasht, char** k, size_t len)
{
	if(len < hasht->e_num)
		return;
	int ki = 0; //Index to the current string in **k
	int i = hasht->capacity;
	while(--i >= 0)
	{
		hash_elem_t* e = hasht->table[i];
		while(e)
		{
			k[ki++] = e->key;
			e = e->next;
		}
	}
}

/* 	List values. v should have length equals or greater 
	than the number of stored elements */
void ht_list_values(hashtable_t* hasht, void** v, size_t len)
{
	if(len < hasht->e_num)
		return;
	int vi = 0; //Index to the current string in **v
	int i = hasht->capacity;
	while(--i >= 0)
	{
		hash_elem_t* e = hasht->table[i];
		while(e)
		{
			v[vi++] = e->data;
			e = e->next;
		}
	}
}

/* Iterate through table's elements. */
hash_elem_t* ht_iterate(hash_elem_it* iterator)
{
	while(iterator->elem == NULL)
	{
		if(iterator->index < iterator->ht->capacity - 1)
		{
			iterator->index++;
			iterator->elem = iterator->ht->table[iterator->index];
		}
		else
			return NULL;
	}
	hash_elem_t* e = iterator->elem;
	if(e)
		iterator->elem = e->next;
	return e;
}

/* Iterate through keys. */
char* ht_iterate_keys(hash_elem_it* iterator)
{
	hash_elem_t* e = ht_iterate(iterator);
	return (e == NULL ? NULL : e->key);
}

/* Iterate through values. */
void* ht_iterate_values(hash_elem_it* iterator)
{
	hash_elem_t* e = ht_iterate(iterator);
	return (e == NULL ? NULL : e->data);
}

/* 	Removes all elements stored in the hashtable.
	if free_data, all stored datas are also freed.*/
void ht_clear(hashtable_t* hasht, int free_data)
{
	hash_elem_it it = HT_ITERATOR(hasht);
	char* k = ht_iterate_keys(&it);
	while(k != NULL)
	{
		free_data ? free(ht_remove(hasht, k)) : ht_remove(hasht, k);
		k = ht_iterate_keys(&it);
	}
}

/* 	Destroy the hash table, and free memory.
	Data still stored are freed*/
void ht_destroy(hashtable_t* hasht)
{
	ht_clear(hasht, 1); // Delete and free all.
	free(hasht->table);
	free(hasht);
}

/*****************************************
 *            FIFO                       *
 *****************************************/

fifo_t* fifo_create()
{
	fifo_t* fifo = malloc(sizeof(fifo_t));
	if(fifo == NULL)
		return NULL;
	fifo_init(fifo);
	return fifo;
}

void fifo_init(fifo_t* fifo)
{
	fifo->fifo_first = NULL;
	fifo->fifo_last = NULL;
	fifo->size = 0;
}

int fifo_push(fifo_t *fifo, void* elem)
{
	fifo_elem_t* new = malloc(sizeof(fifo_elem_t));
	if(new == NULL)
		return -1;
	new->ptr = elem;
	new->next = NULL;

	fifo_elem_t* last = fifo->fifo_last;
	if(fifo->size == 0)
	{
		fifo->fifo_first = new;
		fifo->fifo_last = new;
	}
	else
	{
		fifo->fifo_last = new;
		last->next = new;
	}
	fifo->size ++;
	return 0;
}

void* fifo_pop(fifo_t *fifo)
{
	fifo_elem_t* first = fifo->fifo_first;
	if(fifo->size > 0)
	{
		void* ptr = first->ptr;
		fifo->fifo_first = first->next;
		if(fifo->fifo_last == first)
			fifo->fifo_last = NULL;
		free(first);
		fifo->size --;
		return ptr;
	}
	return NULL;
}

void fifo_destroy(fifo_t *fifo, int free_element)
{
	void* ptr;
	while(fifo->size > 0)
	{
		ptr = fifo_pop(fifo);
		if(free_element)
			free(ptr);
	}
	free(fifo);
}

/*****************************************
 *            LINKED LIST                *
 *****************************************/

linked_list_t* linked_list_create()
{
	linked_list_t* list = malloc(sizeof(linked_list_t));
	if(list == NULL)
		return NULL;
	linked_list_init(list);
	return list;
}

void linked_list_init(linked_list_t* list)
{
	list->first = NULL;
	list->size = 0;
}

void linked_list_add(linked_list_t* list, void* ptr)
{
	linked_elem_t* elem = malloc(sizeof(linked_elem_t));
	elem->ptr = ptr;
	elem->next = list->first;
	list->first = elem;
	list->size ++;
}

void* linked_list_get(linked_list_t* list, int index)
{
	int i= 0;
	if(index >= list->size)
		return NULL;
	linked_elem_t* elem = list->first;
	while(i < list->size)
	{
		if(i == index)
			return elem->ptr;
		elem = elem->next;
		i++;
	}
	return NULL;
}

void* linked_list_iterate(linked_list_t* list, void** iterator)
{
	linked_elem_t *elem;
	if(list != NULL)
		elem = list->first;
	else
		elem = (linked_elem_t*)*iterator;

	if(elem != NULL)
		*iterator = elem->next;

	return (elem == NULL ? NULL : elem->ptr);
}

void* linked_list_remove(linked_list_t* list, int index)
{
	int i= 0;
	if(index >= list->size)
		return NULL;
	linked_elem_t* elem = list->first;
	linked_elem_t* prev = NULL;
	while(i < list->size && elem != NULL)
	{
		if(i == index)
		{
			if(prev != NULL)
				prev->next = elem->next;
			if(list->first == elem)
				list->first = elem->next;
			void* ret = elem->ptr;
			free(elem);
			list->size --;
			return ret;
		}
		prev = elem;
		elem = elem->next;
		i++;
	}
	return NULL;
}

void linked_list_clean(linked_list_t* list, int free_data)
{
	while(list->size > 0)
		free_data ? free(linked_list_remove(list, 0)) : linked_list_remove(list, 0);
}

void linked_list_destroy(linked_list_t* list, int free_data)
{
	linked_list_clean(list, free_data);
	free(list);
}

