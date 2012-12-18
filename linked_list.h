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
 * linked_list.h
 *
 *  Created on: 30 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

typedef struct linked_elem_t {
	struct linked_elem_t* next;
	struct linked_elem_t* prev;
	void* ptr;
}linked_elem_t;

typedef struct {
	linked_elem_t* first;
	linked_elem_t* last;
	int size;
}linked_list_t;

linked_list_t* linked_list_create();

void linked_list_init(linked_list_t* list);

void linked_list_append(linked_list_t* list, void* ptr);

void* linked_list_get(linked_list_t* list, int index);

void* linked_list_iterate(linked_list_t* list, void** iterator);

void linked_list_remove(linked_list_t* list, int index, int free_data);

void linked_list_clean(linked_list_t* list, int free_data);

void linked_list_destroy(linked_list_t* list, int free_data);

#endif /* LINKED_LIST_H_ */
