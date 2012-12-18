/*
 * utils.h
 *
 *  Created on: 12 août 2012
 *      Author: phsymo10
 */

#ifndef UTILS_H_
#define UTILS_H_

#define false 0x00
#define true 0x01

typedef unsigned char bool;

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
extern int MAX_DEBUG_LEVEL;

extern char* DBG_LVL_STR[];

void _log(DBG_LVL level, char* message, ...);

void _perror(char* message, ...);

//MinGW doesn't know strtok_r
#ifdef __MINGW32__
char* strtok_r(char* ori, char* tok, char** it);
#endif

#endif /* UTILS_H_ */
