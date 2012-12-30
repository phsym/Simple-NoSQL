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
 * indextable.h
 *
 *  Created on: 30 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef INDEXTABLE_H_
#define INDEXTABLE_H_

#include "linked_list.h"

typedef struct {
	int capacity;
	int keys_num;
	linked_list_t* lists;
}index_table_t;

typedef struct {
	int ptr;
	char key[];
}index_t;

index_table_t* index_table_create(int capacity);

void index_table_put(index_table_t* table, char* key, int ptr);

int index_table_get(index_table_t* table, char* key);

void index_table_remove(index_table_t* table, char* key);

void index_table_clean(index_table_t* table);

void index_table_destroy(index_table_t* table);

int index_table_keys_number(index_table_t* table);

int index_table_count_keys(index_table_t* table);

void index_table_list_keys(index_table_t* table, char** keys, int len);

//void index_table_rehash(index_table_t* table, int new_capacity);

unsigned int hash(char* str, int str_len);

#endif /* INDEXTABLE_H_ */
