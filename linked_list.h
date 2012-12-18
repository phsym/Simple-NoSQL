/*
 * linked_list.h
 *
 *  Created on: 30 juil. 2012
 *      Author: phsymo10
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
