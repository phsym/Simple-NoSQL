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
 * crypto.c
 *
 *  Created on: 16 janv. 2013
 *      Author: Pierre-Henri Symoneaux
 */

#include "crypto.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "utils.h"

const hash_algo_t hash_a[] = {
		{"MD5", &MD5},
		{"SHA1", &SHA1},
		{"SHA256", &SHA256}
};

hashtable_t *hash_algo_dict;

void crypto_init()
{
	hash_algo_dict = hashtable_create(128);
	int n = sizeof(hash_a)/sizeof(hash_algo_t);
	int i;
	for(i = 0; i < n; i++)
		crypto_register_hash_algo(hash_a +i);
}

void crypto_register_hash_algo(hash_algo_t* algo)
{
	_log(LVL_DEBUG, "Registering hash algorithm %s\n", algo->name);
	hashtable_put(hash_algo_dict, algo->name, algo);
}
