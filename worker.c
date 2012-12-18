/*
 * worker.c
 *
 *  Created on: 28 juil. 2012
 *      Author: phsymo10
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
