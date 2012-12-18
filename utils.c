/*
 * utils.c
 *
 *  Created on: 12 août 2012
 *      Author: phsymo10
 */

#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

DBG_LVL DEBUG_LEVEL = LVL_DEBUG;

char* DBG_LVL_STR[] = {"NONE", "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "TRACE"};
int MAX_DEBUG_LEVEL = 7;

void _log(DBG_LVL level, char* message,  ...)
{
	if(level > DEBUG_LEVEL || level == LVL_NONE)
		return;

	va_list argptr;
	va_start(argptr,message);
	char mess[strlen(message) + strlen(DBG_LVL_STR[level]) + 3];
	mess[0] = '\0';
	strcat(mess, DBG_LVL_STR[level]);
	strcat(mess, "\t: ");
	strcat(mess, message);
	if(level <= LVL_ERROR)
		vfprintf(stderr, mess, argptr);
	else
		vprintf(mess, argptr);
	va_end(argptr);
}

void _perror(char* message, ...)
{
	va_list argptr;
	va_start(argptr,message);
	vfprintf(stderr, message, argptr);
	fprintf(stderr, " : %s\n", sys_errlist[errno]);
	va_end(argptr);
}

//MinGW doesn't know strtok_r
#ifdef __MINGW32__
char* strtok_r(char* ori, char* tok, char** it)
{
	char token = tok[0];
	
	if(ori != NULL)
		*it = ori; // TODO: string copy
		// strcpy(*it, ori);
	char* old_it = *it;
	char* c = strchr(*it, token);
	if(c != NULL)
	{
		*c = '\0';
		*it = (c + 1);
	}
	return old_it;
}
#endif
