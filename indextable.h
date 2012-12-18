/*
 * indextable.h
 *
 *  Created on: 30 juil. 2012
 *      Author: phsymo10
 */

#ifndef INDEXTABLE_H_
#define INDEXTABLE_H_

#include "linked_list.h"

typedef struct {
	int capacity;
	linked_list_t* lists;
}index_table_t;

typedef struct {
	char *key;
	int ptr;
}index_t;

index_table_t* index_table_create(int capacity);

void index_table_put(index_table_t* table, char* key, int ptr);

int index_table_get(index_table_t* table, char* key);

void index_table_remove(index_table_t* table, char* key);

void index_table_clean(index_table_t* table);

void index_table_destroy(index_table_t* table);

int index_table_count_keys(index_table_t* table);

void index_table_list_keys(index_table_t* table, char** keys, int len);

//void index_table_rehash(index_table_t* table, int new_capacity);

unsigned int hash(char* str, int str_len);

#endif /* INDEXTABLE_H_ */
