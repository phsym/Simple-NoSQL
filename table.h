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
 * table.h
 *
 *  Created on: 28 juil. 2012
 *      Author: Pierre-Henri Symoneaux
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
