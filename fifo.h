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
 * fifo.h
 *
 *  Created on: 27 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef FIFO_H_
#define FIFO_H_

// TODO : Use a fix size pointer array ???

typedef struct fifo_elem_t{
	struct fifo_elem_t* next;
	void* ptr;
}fifo_elem_t;

typedef struct {
	fifo_elem_t* fifo_first;
	fifo_elem_t* fifo_last;
	int size;

}fifo_t;

void fifo_append(fifo_t *fifo, void* elem);

void* fifo_pop(fifo_t *fifo);

fifo_t* fifo_create();

void fifo_init(fifo_t* fifo);

void fifo_destroy(fifo_t *fifo, int free_element);


#endif /* FIFO_H_ */
