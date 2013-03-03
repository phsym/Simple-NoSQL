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
 * datatypes.h
 *
 *  Created on: 3 mars 2013
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <stdint.h>


#define TYPE_INT 	1
#define TYPE_DOUBLE 2
#define TYPE_STRING 3
#define TYPE_BLOB 	4

typedef struct {
	unsigned char type;
	unsigned char value[];
} type_generic;

typedef struct {
	unsigned char type;
	int64_t value;
} type_int;

typedef struct {
	unsigned char type;
	double value;
} type_double;

typedef struct {
	unsigned char type;
	uint lenth;
	char value[];
} type_string;

#define type_string_dt(vs) struct { \
								unsigned char type; \
								uint32_t lenth; \
								char value[vs]; \
							}

typedef struct {
	unsigned char type;
	uint64_t size;
	unsigned char value[];
} type_blob;

#define type_blob_dt(vs) struct { \
								unsigned char type; \
								uint64_t size; \
								unsigned char value[vs]; \
							}

#endif /* DATATYPES_H_ */
