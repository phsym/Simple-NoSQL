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
 * linked_list.c
 *
 *  Created on: 30 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

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
	list->last = NULL;
	list->size = 0;
}

void linked_list_append(linked_list_t* list, void* ptr)
{
	linked_elem_t* elem = malloc(sizeof(linked_elem_t));
	elem->ptr = ptr;
	elem->next = NULL;

	if(list->size == 0)
	{
		elem->prev = NULL;
		list->first = elem;
		list->last = elem;
	}
	else
	{
		elem->prev = list->last;
		list->last->next = elem;
		list->last = elem;
	}
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

void linked_list_remove(linked_list_t* list, int index, int free_data)
{
	int i= 0;
	if(index >= list->size)
		return;
	linked_elem_t* elem = list->first;
	while(i < list->size && elem != NULL)
	{	
		if(i == index)
		{
			if(elem->next != NULL)
				elem->next->prev = elem->prev;
			if(elem->prev != NULL)
				elem->prev->next = elem->next;
			if(list->first == elem)
				list->first = elem->next;
			if(list->last == elem)
				list->last = elem->prev;
			if(free_data)
				free(elem->ptr);
			free(elem);
			list->size --;
			return;
		}
		elem = elem->next;
		i++;
	}
}

void linked_list_clean(linked_list_t* list, int free_data)
{
	while(list->size > 0)
		linked_list_remove(list, 0, free_data);
}

void linked_list_destroy(linked_list_t* list, int free_data)
{
	linked_list_clean(list, free_data);
	free(list);
}
