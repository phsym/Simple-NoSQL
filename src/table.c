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
 * table.c
 *
 *  Created on: 28 juil. 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include "table.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef __MINGW32__
	#include <windows.h>
#else
	#include <sys/mman.h>
#endif
	
void table_init(table_t* table, uint64_t data_size, uint64_t capacity)
{
	table->magic = TABLE_MAGIC;
	table->blk_size = data_size + sizeof(table_elem_t);
	table->data_size = data_size;
	table->capacity = capacity;
	table->first_free = 0;

	uint64_t i;
	uint64_t p;
	for(i = 0; i < capacity; i++)
	{
		p = (i*100)/capacity;
		if(i%(capacity/100) == 0)
		{
			_log(LVL_INFO, "Initializing table : %d%%\r", p);
			fflush(stdout);
		}
		table_elem_t *elem = (void*)table->table + i * table->blk_size;
		memset(elem->data, 0, data_size);
		elem->flag = FLAG_NONE;
		if(i < capacity-1)
			elem->ind = i+1;
		else
			elem->ind = -1;
	}
	_log(LVL_INFO, "Initializing table : %d%%\r\n", (i*100)/capacity);
}

table_t* table_map_create(char* filename, uint64_t data_size, uint64_t capacity)
{
	int fd = open(filename, O_RDWR|O_CREAT, (mode_t)0600);
	if (fd < 0)
		return NULL;
	uint64_t size = sizeof(table_t) + (capacity * (data_size + sizeof(table_elem_t)));

	char val = 0;

#ifdef __MINGW32__
	if((lseek64(fd, size-1, SEEK_SET) < 0) || (write(fd, &val, 1) < 0))
#else
	if((lseek(fd, size-1, SEEK_SET) < 0) || (write(fd, &val, 1) < 0))
#endif
	{
		_log(LVL_ERROR, "Could not create file. Maybe not enough space on disk or file too big\n");
		close(fd);
		remove(filename);
		return NULL;
	}
	write(fd, &val, 1);
#ifdef __MINGW32__
	FlushFileBuffers ((HANDLE) _get_osfhandle (fd));
#else
	fsync(fd);
#endif
	
	//init table
	_log(LVL_INFO, "Mapping data table file ...\n");
#ifdef __MINGW32__
	HANDLE mapping = CreateFileMapping((HANDLE) _get_osfhandle (fd), NULL, PAGE_READWRITE, 0, 0, NULL);
	table_t *table = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#else
	table_t *table = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#endif
	if(table == (void*)-1 || table == NULL)
	{
		_log(LVL_ERROR, "Could not map table. Not enough memory maybe.\n");
#ifdef __MINGW32__
		CloseHandle(mapping);
#endif
		close(fd);
		if(remove(filename) < 0)
			_perror("");
		return NULL;
	}

	table_init(table, data_size, capacity);

	// _log(LVL_INFO, "Syncing data table file (this may take a while) ...\n");
#ifdef __MINGW32__
	//TODO : msync
#else
	_log(LVL_INFO, "Syncing data table file (this may take a while) ...\n");
	msync(table, size, MS_SYNC);
#endif

	return table;
}

table_t* table_create(uint64_t data_size, uint64_t capacity)
{
	uint64_t size = sizeof(table_t) + (capacity * (data_size + sizeof(table_elem_t)));

	//init table
	table_t* table = malloc(size);
	//Check if table has been allocated
	if(table == NULL)
		return NULL;

	table_init(table, data_size, capacity);

	return table;
}

table_t* table_map_load(char* filename)
{
	_log(LVL_INFO, "Loading data table file ...\n"); 
	int fd = open(filename, O_RDWR);
	if (fd < 0)
		return NULL;
	table_t t;
	int r = read(fd, &t, sizeof(t));
	if(r != sizeof(t) || t.magic != TABLE_MAGIC)
		return NULL;
	uint64_t size = sizeof(table_t) + (t.capacity * (t.data_size + sizeof(table_elem_t)));

	_log(LVL_INFO, "Mapping data table file ...\n"); 
#ifdef __MINGW32__
	HANDLE mapping = CreateFileMapping((HANDLE) _get_osfhandle (fd), NULL, PAGE_READWRITE, 0, 0, NULL);
	table_t *table = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#else
	table_t *table = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#endif
	if(table == (void*)-1 || table == NULL)
	{
		_log(LVL_ERROR, "Could not map table. Not enough memory maybe.\n");
#ifdef __MINGW32__
		CloseHandle(mapping);
#endif
		close(fd);
		return NULL;
	}
	return table;
}

uint64_t* table_put(table_t* table, void* data)
{
	uint64_t index = table->first_free;
	table_elem_t *e = ((void*)table->table + index * table->blk_size);
	table->first_free = e->ind;
	e->ind = index;
	memcpy(e->data, data, table->data_size);
	e->flag |= FLAG_USED;
	return &e->ind;
}

table_elem_t* table_get_block(table_t* table, uint64_t index)
{
	if(index >= table->capacity)
			return NULL;
	table_elem_t *e = ((void*)table->table + index * table->blk_size);
	if((e->flag & FLAG_USED) == 0)
		return NULL;
	return e;
}

void* table_get_ref(table_t* table, uint64_t index)
{
	if(index >= table->capacity)
		return NULL;
	table_elem_t *e = ((void*)table->table + index * table->blk_size);
	if((e->flag & FLAG_USED) == 0)
		return NULL;
	return e->data;
}

void table_get_copy(table_t* table, uint64_t index, void* ptr)
{
	if(index >= table->capacity)
		ptr = NULL;
	table_elem_t *e = ((void*)table->table + index * table->blk_size);
	if((e->flag & FLAG_USED) == 0)
		ptr = NULL;
	memcpy(ptr, e->data, table->data_size);
}

void table_remove(table_t* table, uint64_t index)
{
	if(index < table->capacity)
	{
		table_elem_t *e = ((void*)table->table + index * table->blk_size);
		if((e->flag & FLAG_USED) == 0)
			return;
		e->flag = FLAG_NONE;
		e->ind = table->first_free;
		table->first_free = index;
	}
}

void table_clean(table_t* table, uint64_t index)
{
	if(index < table->capacity)
		memset(((void*)table->table + index * table->blk_size), 0, table->blk_size);
}

void destroy_map_table(table_t* table)
{
	uint64_t size = sizeof(table_t) + (table->capacity * (table->data_size + sizeof(table_elem_t)));

	_log(LVL_INFO, "Syncing data table file ...\n");
#ifdef __MINGW32__
	//TODO : msync
#else
	msync(table, size, MS_SYNC);
#endif

	_log(LVL_INFO, "Unmapping data table file ...\n");
#ifdef __MINGW32__
	UnmapViewOfFile(table);
#else
	munmap(table, size);
#endif

	//TODO : Close file
}

void destroy_table(table_t* table)
{
	free(table);
}

void table_resize(table_t* table, uint64_t new_capacity)
{
	table = realloc(table, sizeof(table_t) + (new_capacity * table->blk_size));
	//TODO : Modify free block ptr
}
