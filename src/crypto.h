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
 * crypto.h
 *
 *  Created on: 16 janv. 2013
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <stdint.h>

#include "containers.h"

typedef struct {
	char* name;
	size_t digest_len;
	size_t digest_str_len;
	unsigned char* (*hash_func)(const unsigned char *m, uint32_t len, unsigned char *md);
} hash_algo_t;

extern hashtable_t *hash_algo_dict;

void crypto_init();
void crypto_cleanup();
void crypto_register_hash_algo(hash_algo_t* algo);
hash_algo_t* crypto_get_hash_algo(char* algo_name);

void crypto_hash_to_str(const hash_algo_t* algo, unsigned char *d, char* str);
void crypto_hash_str(const hash_algo_t* algo, const char *M, uint32_t len, char* digest_str);

#endif /* CRYPTO_H_ */
