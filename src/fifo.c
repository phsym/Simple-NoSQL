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
 * fifo.c
 *
 *  Created on: 27 juil. 2012
 *      Author: Pierre-Henri Symoneaux
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
