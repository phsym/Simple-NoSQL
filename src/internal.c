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
 * internal.c
 *
 *  Created on: 28 janv. 2013
 *      Author: Author: Pierre-Henri Symoneaux
 */

#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "crypto/crypto.h"

uint64_t intern_get_storage_size(dbs_t* dbs, char* dbname)
{
	char tmp_k[2048];
	CAT4(tmp_k, PREFIX_DB, dbname, SUFFIX_DB_STORAGE_SIZE);
	char* size = datastore_lookup(dbs->intern_db, tmp_k);
	uint64_t ret = (size != NULL ? strtoul(size, NULL, 10) : 0);
	if(size != NULL)
		free(size);
	return ret;

}

uint64_t intern_get_storage_index_len(dbs_t* dbs, char* dbname)
{
	char tmp_k[2048];
	CAT4(tmp_k, PREFIX_DB, dbname, SUFFIX_DB_INDEX_SIZE);
	char* len = datastore_lookup(dbs->intern_db, tmp_k);
	uint64_t ret = (len != NULL ? strtoul(len, NULL, 10) : 0);
	if(len != NULL)
		free(len);
	return ret;
}

int intern_set_storage_size(dbs_t* dbs, char* dbname, char* size)
{
	char tmp_k[2048];
	CAT4(tmp_k, PREFIX_DB, dbname, SUFFIX_DB_STORAGE_SIZE);
	return datastore_put(dbs->intern_db, tmp_k, size);
}

int intern_set_storage_index_len(dbs_t* dbs, char* dbname, char* len)
{
	char tmp_k[2048];
	CAT4(tmp_k, PREFIX_DB, dbname, SUFFIX_DB_INDEX_SIZE);
	return datastore_put(dbs->intern_db, tmp_k, len);
}

int intern_set_default_db(dbs_t* dbs, char* dbname)
{
	datastore_t * store = ht_get(dbs->storages, dbname);
	if(store != NULL)
	{
		datastore_set(dbs->intern_db, INT_DEFAULT_DB, dbname);
		_log(LVL_INFO, "Default databased changed to %s\n", dbname);
		return 0;
	}
	else
	{
		_log(LVL_ERROR, "Could not set DB %s which does not exists to default\n", dbname);
		return -1;
	}
}

datastore_t* intern_get_default_db(dbs_t* dbs)
{
	char* def_db = datastore_lookup(dbs->intern_db, INT_DEFAULT_DB);
	datastore_t* ret = NULL;
	if(def_db != NULL)
	{
		ret = ht_get(dbs->storages, def_db);
		free(def_db);
	}
	return ret;
}

void intern_load_storages(dbs_t* dbs)
{
	char* tmp = datastore_lookup(dbs->intern_db, INT_DB_LIST);
	if(tmp != NULL)
	{
		char db_l[strlen(tmp) + 1];
		strcpy(db_l, tmp);
		free(tmp);
		char* str;
		char* db = strtok_r(db_l, " ", &str);
		while(db != NULL)
		{
			//Load storage and index size from internal db
			_log(LVL_INFO, "Loading DB %s\n", db);
			uint64_t stor_len = intern_get_storage_size(dbs, db);
			uint64_t index_len = intern_get_storage_index_len(dbs, db);

			datastore_t* store = datastore_create(db, stor_len, index_len);
			if(store != NULL)
				ht_put(dbs->storages, store->name, store);
			db = strtok_r(NULL, " ", &str);
		}
	}
}

int intern_create_new_db(dbs_t* dbs, char* dbname, char* store_size, char* index_len)
{
	datastore_t * store = ht_get(dbs->storages, dbname);
	if(store != NULL)
	{
		_log(LVL_ERROR, "Could not create DB %s because it already exists\n", dbname);
		return -1;
	}
	else
	{
		store = datastore_create(dbname, strtoul(store_size, NULL, 10), strtoul(index_len, NULL, 10));
		if(store != NULL)
		{
			_log(LVL_INFO, "Creating db %s\n", dbname);
			char* db_names = datastore_lookup(dbs->intern_db, INT_DB_LIST);
			if(db_names == NULL)
			{
				datastore_put(dbs->intern_db, INT_DB_LIST, "");
				db_names = datastore_lookup(dbs->intern_db, INT_DB_LIST);
			}
			db_names = realloc(db_names, strlen(db_names) + strlen(dbname) + 2);
			strcat(db_names, " ");
			strcat(db_names, dbname);
			datastore_set(dbs->intern_db, INT_DB_LIST, db_names);
			free(db_names);

			intern_set_storage_size(dbs, dbname, store_size);
			intern_set_storage_index_len(dbs, dbname, index_len);
			ht_put(dbs->storages, dbname, store);
			return 0;
		}
		else
		{
			_log(LVL_ERROR, "Could not createdatastore for DB %s\n", dbname);
			return -1;
		}
	}
}

int intern_create_user(dbs_t* dbs, char* username, char* password)
{
	//TODO : Create "exists" function
	char* passwd = intern_get_password(dbs, username);
	if( passwd != NULL)
	{
		free(passwd);
		return -1;
	}
	return intern_set_password(dbs, username, password);
}

int intern_set_password(dbs_t* dbs, char* username, char* password)
{
	char cat[2048];
	CAT4(cat, username, PASSWD_SALT, password);

	hash_algo_t* algo = crypto_get_hash_algo("sha256");
	char digest_str[algo->digest_str_len];
	crypto_hash_str(algo, cat, strlen(cat), digest_str);
	CAT4(cat, PREFIX_USER, username, SUFFIX_USER_PASSWD);
	if(datastore_set(dbs->intern_db, cat, digest_str) < 0)
	{
		_log(LVL_ERROR, "Could not change password\n");
		return -1;
	}
	else
	{
		_log(LVL_INFO, "Password changed for use %s\n", username);
		return 0;
	}
}

char* intern_get_password(dbs_t* dbs, char* username)
{
	char cat[2048];
	CAT4(cat, PREFIX_USER, username, SUFFIX_USER_PASSWD);
	return datastore_lookup(dbs->intern_db, cat);
}

bool intern_verify_credentials(dbs_t* dbs, char* username, char* password)
{
	char cat[128];
	char* auth_tok = intern_get_password(dbs, username);
	if(auth_tok == NULL)
		return false;
	CAT4(cat, username, PASSWD_SALT, password);
	hash_algo_t* algo = crypto_get_hash_algo("sha256");
	char digest_str[algo->digest_str_len];
	crypto_hash_str(algo, cat, strlen(cat), digest_str);

	_log(LVL_TRACE, "Auth token : %s\n", digest_str);

	if(!strcmp(digest_str, auth_tok))
		return true;
	free(auth_tok);
	return false;
}
