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

#define TABLE_ELEMENT(table, index) ((void*)table->table + index * table->blk_size)
	
void table_init(table_t* table, uint64_t data_frag_size, uint64_t capacity)
{
	table->magic = TABLE_MAGIC;
	table->blk_size = data_frag_size + sizeof(table_elem_t);
	table->data_frag_size = data_frag_size;
	table->capacity = capacity;
	table->first_free = 0;

	uint64_t i;
	uint64_t p;
	for(i = 0; i < capacity; i++)
	{
		p = (i*100)/capacity;
		if(capacity >= 100 && i%(capacity/100) == 0)
		{
			_log(LVL_INFO, "Initializing table : %d%%\r", p);
			fflush(stdout);
		}
		table_elem_t *elem = (void*)table->table + i * table->blk_size;
		memset(elem->data, 0, data_frag_size);
		elem->flag = FLAG_NONE;
		if(i < capacity-1)
			elem->ind = i+1;
		else
			elem->ind = -1;
	}
	_log(LVL_INFO, "Initializing table : %d%%\r\n", (i*100)/capacity);
}

table_t* table_map_create(char* filename, uint64_t data_frag_size, uint64_t capacity)
{
	int fd = open(filename, O_RDWR|O_CREAT, (mode_t)0600);
	if (fd < 0)
		return NULL;
	uint64_t size = sizeof(table_t) + (capacity * (data_frag_size + sizeof(table_elem_t)));

	char val = 0;

	if((lseek(fd, size-1, SEEK_SET) < 0) || (write(fd, &val, 1) < 0))
	{
		_log(LVL_ERROR, "Could not create file. Maybe not enough space on disk or file too big\n");
		close(fd);
		remove(filename);
		return NULL;
	}

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

	table_init(table, data_frag_size, capacity);

	// _log(LVL_INFO, "Syncing data table file (this may take a while) ...\n");
#ifdef __MINGW32__
	//TODO : msync
#else
	_log(LVL_INFO, "Syncing data table file (this may take a while) ...\n");
	msync(table, size, MS_SYNC);
#endif

	return table;
}

table_t* table_create(uint64_t data_frag_size, uint64_t capacity)
{
	uint64_t size = sizeof(table_t) + (capacity * (data_frag_size + sizeof(table_elem_t)));

	//init table
	table_t* table = malloc(size);
	//Check if table has been allocated
	if(table == NULL)
		return NULL;

	table_init(table, data_frag_size, capacity);

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
	uint64_t size = sizeof(table_t) + (t.capacity * (t.data_frag_size + sizeof(table_elem_t)));

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

uint64_t* table_put(table_t* table, void* data, uint64_t size)
{
	uint64_t index = table->first_free;
	table_elem_t *e = NULL;

	uint64_t ind = index;

	uint64_t i = 0;
	while(i < size)
	{
		e = TABLE_ELEMENT(table, ind);
		if(i == 0)
		{
			if(((e->flag & FLAG_HEAD) == 0) && (e->flag != FLAG_NONE))
				return NULL;
			e->flag |= FLAG_HEAD;
		}
		e->flag |= FLAG_FRAG;
		memcpy(e->data, data + i, MIN(size - i, table->data_frag_size));

		ind = e->ind;
		i+= MIN(size - i, table->data_frag_size);
	}

	table->first_free = e->ind;
	e->ind = index;
	e->flag |= FLAG_END;
	return &e->ind;
}

table_elem_t* table_get_block(table_t* table, uint64_t index)
{
	if(index >= table->capacity)
		return NULL;
	table_elem_t *e = TABLE_ELEMENT(table, index);
	if((e->flag & FLAG_FRAG) == 0)
		return NULL;
	return e;
}

int table_get_copy(table_t* table, uint64_t index, void* ptr, uint64_t size)
{
	if(index >= table->capacity)
		return 0;

	table_elem_t *e = TABLE_ELEMENT(table, index);

	if((e->flag & FLAG_HEAD) == 0)
		return 0;

	uint64_t i = 0;
	while(i < size)
	{
		memcpy(ptr+i, e->data, MIN(table->data_frag_size, size - i));
		i+= MIN(table->data_frag_size, size - i);
		if(e->flag & FLAG_END)
			break;

		e = TABLE_ELEMENT(table, e->ind);
	}
	return i;
}

void table_remove(table_t* table, uint64_t index)
{
	if(index < table->capacity)
	{
		table_elem_t *e = TABLE_ELEMENT(table, index);
		if((e->flag & FLAG_HEAD) == 0)
			return;

		while(1)
		{
			if((e->flag & FLAG_END) != 0)
			{
				e->flag = FLAG_NONE;
				break;
			}
			e->flag = FLAG_NONE;
			e = TABLE_ELEMENT(table, e->ind);
		}

		e->ind= table->first_free;
		table->first_free = index;
	}
}

void destroy_map_table(table_t* table)
{
	uint64_t size = sizeof(table_t) + (table->capacity * (table->data_frag_size + sizeof(table_elem_t)));

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
