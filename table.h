/*
 * table.h
 *
 *  Created on: 28 juil. 2012
 *      Author: phsymo10
 */

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#define TABLE_MAGIC 0x6F

#define ELEM_UNUSED 0x00
#define ELEM_USED	0x01

typedef struct {
	unsigned char used;
	int next_free;
	char data[];
}table_elem_t;

typedef struct {
	char magic;
	int data_size;
	int blk_size; // data block size = data_size + sizeof table_elem_t
	int capacity;
	int first_free;
	table_elem_t table[];
}table_t;

void table_init(table_t* table, int data_size, int capacity);

table_t* table_map_create(char* filename, int data_size, int capacity);

table_t* table_create(int data_size, int capacity);

table_t* table_map_load(char* filename);

int table_put(table_t* table, void* data);

void* table_get_ref(table_t* table, int index);

void table_get_copy(table_t* table, int index, void* ptr);

void table_remove(table_t* table, int index);

void table_clean(table_t* table, int index);

void destroy_map_table(table_t* table);

void destroy_table(table_t* table);

void table_resize(table_t* table, int new_capacity);

#endif /* HASHTABLE_H_ */
