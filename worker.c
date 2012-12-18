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
 * worker.c
 *
 *  Created on: 28 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include <stdlib.h>

#include "worker.h"

TH_HDL worker_handler(void* worker)
{
	((worker_t*) worker)->running = true;
	((worker_t*) worker)->handler(((worker_t*) worker));
	TH_RETURN;
}

workers_pool_t* workers_pool_create(int size, void* shared_data, void (*handler)(struct worker_t*))
{
	workers_pool_t* pool = malloc(sizeof(workers_pool_t));
	if(pool == NULL)
		return NULL;
	pool->running = false;
	pool->num_workers = size;
	pool->shared_data = shared_data;
	int i;
	pool->workers = malloc(pool->num_workers * (sizeof(worker_t*)));
	for(i = 0; i < pool->num_workers; i++)
		*(pool->workers + i)  = worker_create(pool, handler);
	return pool;
}

void workers_pool_destroy(workers_pool_t* pool)
{
	int i;
	if(pool->running)
		workers_pool_stop(pool);
	for(i = 0; i < pool->num_workers; i++)
		worker_destroy(*(pool->workers + i));
	free(pool->workers);
	free(pool);
}

void workers_pool_start(workers_pool_t* pool)
{
	if(pool->running)
		return;
	pool->running = true;
	int i;
	for(i = 0; i < pool->num_workers; i++)
		worker_start(*(pool->workers + i));
}

void workers_pool_stop(workers_pool_t* pool)
{
	if(!pool->running)
		return;
	pool->running = false;
	int i;
	for(i = 0; i < pool->num_workers; i++)
		worker_stop(*(pool->workers + i));
}

worker_t* worker_create(workers_pool_t* pool, void (*handler)(struct worker_t*))
{
	worker_t* worker = malloc(sizeof(worker_t));
	if(worker == NULL)
		return NULL;
	worker->pool = pool;
	worker->running = false;
	worker->handler = handler;
	return worker;
}

void worker_destroy(worker_t* worker)
{
	if(worker->running)
		worker_stop(worker);
	free(worker);
}

void worker_start(worker_t* worker)
{
	thread_create(&worker->thread, &worker_handler, worker, 0);
}

void worker_stop(worker_t* worker)
{
	worker->running = false;
}
