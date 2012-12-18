/*
 * fifo.h
 *
 *  Created on: 27 juil. 2012
 *      Author: phsymo10
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
