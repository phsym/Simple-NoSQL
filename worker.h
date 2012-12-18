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
 * worker.h
 *
 *  Created on: 28 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef WORKER_H_
#define WORKER_H_

#include "concurrency.h"
#include "protocol.h"
#include "datastorage.h"
#include "utils.h"

typedef struct workers_pool_t workers_pool_t;

typedef struct worker_t{
	volatile bool running;
	thread_t thread;
	workers_pool_t* pool;
	void (*handler)(struct worker_t*);
} worker_t;

struct workers_pool_t {
	bool running;
	worker_t **workers;
	int num_workers;
	void* shared_data;
};

TH_HDL worker_handler(void* worker);

workers_pool_t* workers_pool_create(int size, void* shared_data, void (*handler)(struct worker_t*));

void workers_pool_destroy(workers_pool_t* pool);

void workers_pool_start(workers_pool_t* pool);

void workers_pool_stop(workers_pool_t* pool);

worker_t* worker_create(workers_pool_t* pool,void (*handler)(struct worker_t*));

void worker_destroy(worker_t* worker);

void worker_start(worker_t* worker);

void worker_stop(worker_t* worker);

#endif /* WORKER_H_ */
