/*
 * datastorage.h
 *
 *  Created on: 3 août 2012
 *      Author: phsymo10
 */

#ifndef DATASTORAGE_H_
#define DATASTORAGE_H_

#include "concurrency.h"
#include "table.h"
#include "indextable.h"

typedef struct {
	char name[32];
	char value[32];
}data_t;

typedef struct {
	index_table_t *index_table;
	table_t *data_table;
	semaphore_t semaphore;
}datastore_t;

datastore_t* datastore_create(int storage_size, int index_length);

char* datastore_lookup(datastore_t* datastore, char* key);

int datastore_put(datastore_t* datastore, char* key, char* value);

int datastore_remove(datastore_t* datastore, char* key);

int datastore_count_keys(datastore_t* datastore);

void datastore_list_keys(datastore_t* datastore, char **keys, int len);

void datastore_destroy(datastore_t* datastore);

#endif /* DATASTORAGE_H_ */
