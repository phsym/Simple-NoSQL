/*
 * worker.h
 *
 *  Created on: 28 juil. 2012
 *      Author: phsymo10
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
