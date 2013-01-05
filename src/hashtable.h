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
 * hashtable.h
 *
 *  Created on: 30 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdint.h>
#include "linked_list.h"

typedef struct {
	int capacity;
	int keys_num;
	linked_list_t* lists;
}hashtable_t;

typedef struct {
	uintptr_t ptr;
	char key[];
}hashtable_elem_t;

hashtable_t* hashtable_create(int capacity);

void hashtable_put(hashtable_t* table, char* key, uintptr_t ptr);

uintptr_t hashtable_get(hashtable_t* table, char* key);

void hashtable_remove(hashtable_t* table, char* key);

void hashtable_clean(hashtable_t* table);

void hashtable_destroy(hashtable_t* table);

int hashtable_keys_number(hashtable_t* table);

int hashtable_count_keys(hashtable_t* table);

void hashtable_list_keys(hashtable_t* table, char** keys, int len);

//void hashtable_rehash(index_table_t* table, int new_capacity);

unsigned int hash(char* str, int str_len);

#endif /* HASHTABLE_H_ */
