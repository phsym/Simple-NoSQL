/*
 * fifo.c
 *
 *  Created on: 27 juil. 2012
 *      Author: phsymo10
 */

#include <stdlib.h>

#include "fifo.h"

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

void fifo_append(fifo_t *fifo, void* elem)
{
	fifo_elem_t* new = malloc(sizeof(fifo_elem_t));
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
