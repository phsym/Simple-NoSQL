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
 * utils.h
 *
 *  Created on: 12 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <sys/types.h>

//UNUSED macro is for removing warnings about unused parameter
#define UNUSED(x) (void)(x)

#define TIME_STRLEN 18

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

//#define false 0x00
//#define true 0xFF
//
//typedef unsigned char bool;

typedef enum bool {
	false = 0x00,
	true = 0xFF
}bool;

typedef enum DBG_LVL{
	LVL_NONE = 0,
	LVL_FATAL = 1,
	LVL_ERROR = 2,
	LVL_WARNING = 3,
	LVL_INFO = 4,
	LVL_DEBUG = 5,
	LVL_TRACE = 6
}DBG_LVL;

extern DBG_LVL DEBUG_LEVEL;

extern const int MAX_DEBUG_LEVEL;
extern const char* DBG_LVL_STR[];

void _log_init(char* logfile);

// Automatically called at exit
void _log_cleanup();

void _log(DBG_LVL level, char* message, ...);

void _perror(char* message, ...);

void get_current_time_string(char* str, size_t len);

unsigned int get_time();

char* strqtok_r(char* ori, char** it);

//MinGW doesn't know strtok_r
#ifdef __MINGW32__
char* strtok_r(char* ori, char* tok, char** it);
#endif

#endif /* UTILS_H_ */
