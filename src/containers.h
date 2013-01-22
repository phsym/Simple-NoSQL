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
 * containers.h
 *
 *  Created on: 22 janv. 2013
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef CONTAINERS_H_
#define CONTAINERS_H_

//Hashtable element structure
typedef struct hash_elem_t {
	struct hash_elem_t* next; // Next element in case of a collision
	void* data;	// Pointer to the stored element
	char key[]; // Key of the stored element
} hash_elem_t;

//Hashtabe structure
typedef struct {
	unsigned int capacity;	// Hashtable capacity (in terms of hashed keys)
	unsigned int e_num;		// Number of element currently stored in the hashtable
	hash_elem_t** table;	// The table containaing elements
} hashtable_t;

//Structure used for iterations
typedef struct {
	hashtable_t* ht; 	// The hashtable on which we iterate
	unsigned int index; // Current index in the table
	hash_elem_t* elem; 	// Curent element in the list
} hash_elem_it;

#define HT_ITERATOR_INIT {NULL, 0, NULL}

void* HT_ERROR; // Data pointing to HT_ERROR are returned in case of error

/* 	Create a hashtable with capacity 'capacity'
	and return a pointer to it*/
hashtable_t* ht_create(unsigned int capacity);

/* 	Store data in the hashtable. If data with the same key are already stored,
	they are overwritten, and return by the function. Else it return NULL.
	Return HT_ERROR if there are memory alloc error*/
void* ht_put(hashtable_t* hasht, char* key, void* data);

/* Retrieve data from the hashtable */
void* ht_get(hashtable_t* hasht, char* key);

/* 	Remove data from the hashtable. Return the data removed from the table
	so that you can free memory if needed */
void* ht_remove(hashtable_t* hasht, char* key);

/* List keys. k should have length equals or greater than the number of keys */
void ht_list_keys(hashtable_t* hasht, char** k, size_t len);

/* Iterate through table's elements. */
hash_elem_t* ht_iterate(hashtable_t* hasht, hash_elem_it* iterator);

/* Iterate through keys. */
char* ht_iterate_keys(hashtable_t* hasht, hash_elem_it* iterator);

/* Iterate through values. */
void* ht_iterate_values(hashtable_t* hasht, hash_elem_it* iterator);

/* 	Removes all elements stored in the hashtable.
	if free_data, all stores datas are also freed.*/
void ht_clear(hashtable_t* hasht, int free_data);

/* 	Destroy the hash table, and free memory.
	Data still stored are freed*/
void ht_destroy(hashtable_t* hasht);

#endif /* CONTAINERS_H_ */
