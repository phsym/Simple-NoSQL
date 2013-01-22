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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "crypto.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "utils.h"

const hash_algo_t hash_a[] = {
		{"md5", MD5_DIGEST_LENGTH, MD5_DIGEST_STR_LENGTH, &MD5},
		{"sha1", SHA1_DIGEST_LENGTH, SHA1_DIGEST_STR_LENGTH, &SHA1},
		{"sha256", SHA256_DIGEST_LENGTH, SHA256_DIGEST_STR_LENGTH, &SHA256}
};

hashtable_t *hash_algo_dict;

bool _cryp_init = false;

void crypto_init()
{
	if(!_cryp_init)
	{
		hash_algo_dict = ht_create(128);
		int n = sizeof(hash_a)/sizeof(hash_algo_t);
		int i;
		for(i = 0; i < n; i++)
			crypto_register_hash_algo(hash_a +i);
		_cryp_init = true;
	}
}

void crypto_cleanup()
{
	if(_cryp_init)
	{
		ht_destroy(hash_algo_dict);
		_cryp_init = false;
	}
}

void crypto_register_hash_algo(hash_algo_t* algo)
{
	_log(LVL_DEBUG, "Registering hash algorithm %s\n", algo->name);
	ht_put(hash_algo_dict, algo->name, algo);
}

hash_algo_t* crypto_get_hash_algo(char* algo_name)
{
	return (hash_algo_t*)ht_get(hash_algo_dict, algo_name);

}

void crypto_hash_to_str(const hash_algo_t* algo, unsigned char *d, char* str)
{
	unsigned int i;
	char tmp[3];
	str[0] = '\0';
	for (i = 0; i < algo->digest_len; i++) {
		sprintf(tmp, "%02x", d[i]);
		strcat(str, tmp);
	}
}

void crypto_hash_str(const hash_algo_t* algo, const char *M, uint32_t len, char* digest_str)
{
	unsigned char digest[algo->digest_len];
	algo->hash_func((unsigned char*)M, len, digest);
	crypto_hash_to_str(algo, digest, digest_str);
}
