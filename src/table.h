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

#ifndef TABLE_H_
#define TABLE_H_

#include<stdint.h>

//#define _FILE_OFFSET_BITS 64

#define TABLE_MAGIC 0x6F

#define FLAG_NONE 0x00
#define FLAG_USED	0x01

typedef struct {
	unsigned char flag;
	// ind is multi-pupose : When block unused, it points to the next free block, when used it contains the index of the block.
	// A pointer to this value is stored in the index table
	uint64_t ind;
	char data[];
}table_elem_t;

typedef struct {
	char magic;
	uint64_t data_size;
	uint64_t blk_size; // data block size = data_size + sizeof table_elem_t
	uint64_t capacity;
	uint64_t first_free;
	table_elem_t table[];
}table_t;

void table_init(table_t* table, uint64_t data_size, uint64_t capacity);

table_t* table_map_create(char* filename, uint64_t data_size, uint64_t capacity);

table_t* table_create(uint64_t data_size, uint64_t capacity);

table_t* table_map_load(char* filename);

uint64_t* table_put(table_t* table, void* data);

table_elem_t* table_get_block(table_t* table, uint64_t index);

void* table_get_ref(table_t* table, uint64_t index);

void table_get_copy(table_t* table, uint64_t index, void* ptr);

void table_remove(table_t* table, uint64_t index);

void table_clean(table_t* table, uint64_t index);

void destroy_map_table(table_t* table);

void destroy_table(table_t* table);

void table_resize(table_t* table, uint64_t new_capacity);

#endif /* TABLE_H_ */
