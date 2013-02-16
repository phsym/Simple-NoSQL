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
 * utils.c
 *
 *  Created on: 12 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

DBG_LVL DEBUG_LEVEL = LVL_INFO;

const char* DBG_LVL_STR[] = {"NONE", "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "TRACE"};
const int MAX_DEBUG_LEVEL = 7;

FILE* _log_fd = NULL;
bool _log_is_init = false;

void _log_init(char* logfile)
{
	if(!_log_is_init)
	{
		if(logfile == NULL)
			_log_fd = stdout;
		else
			_log_fd = fopen(logfile, "a+");
		atexit(&_log_cleanup);
		_log_is_init = true;
	}
}

// Automatically called at exit
void _log_cleanup()
{
	if(_log_is_init && _log_fd != NULL && _log_fd != stdout)
	{
		fflush(_log_fd);
		fclose(_log_fd);
		_log_fd = NULL;
	}
}

void _log(DBG_LVL level, char* message,  ...)
{
	if(level > DEBUG_LEVEL || level == LVL_NONE)
		return;

	va_list argptr;
	va_start(argptr,message);

	char date[TIME_STRLEN];
	get_current_time_string(date, TIME_STRLEN);

	char mess[strlen(date) + strlen(message) + strlen(DBG_LVL_STR[level]) + 4];
	mess[0] = '\0';
	strcat(mess, "");
	strcat(mess, date);
	strcat (mess, " ");
	strcat(mess, DBG_LVL_STR[level]);
	strcat(mess, "\t: ");
	strcat(mess, message);
	if(level <= LVL_ERROR)
		vfprintf(stderr, mess, argptr);
	vfprintf(_log_fd, mess, argptr);
	va_end(argptr);
}

void _perror(char* message, ...)
{
	if(LVL_ERROR > DEBUG_LEVEL)
		return;
	va_list argptr;
	va_start(argptr,message);

	char date[TIME_STRLEN];
	get_current_time_string(date, TIME_STRLEN);

	char mess[strlen(date) + strlen(message) + strlen(DBG_LVL_STR[LVL_ERROR]) + 4];
	mess[0] = '\0';
	strcat(mess, "");
	strcat(mess, date);
	strcat (mess, " ");
	strcat(mess, DBG_LVL_STR[LVL_ERROR]);
	strcat(mess, "\t: ");
	strcat(mess, message);

	char* error_str = (char*)sys_errlist[errno];
	vfprintf(stderr, mess, argptr);
	vfprintf(stderr, " : %s\n", error_str);
	vfprintf(_log_fd, mess, argptr);
	vfprintf(_log_fd, " : %s\n", error_str);

	va_end(argptr);
}

void get_current_time_string(char* str, size_t len)
{
#ifdef __MINGW32__
	str[0] = '\0';
	//TODO : Check string length
	char buff[TIME_STRLEN];
	_strdate(buff);
	strcat(str, buff);
	strcat(str, " ");
	_strtime(buff);
	strcat(str, buff);
#else
	time_t t;
	time(&t);
	struct tm r;
	gmtime_r(&t, &r);

	strftime(str, len, "%D %T", &r);
#endif
}

unsigned int get_time()
{
	return time(NULL);
}

char* strqtok_r(char* ori, char** it)
{
	if(ori != NULL)
		*it = ori;
	if(*it == NULL)
		return NULL;

	char* o;
	while(**it == ' ')
		(*it)++;
	if(**it == '\"')
		o = strtok_r(NULL, "\"", it);
	else if(**it == '\'')
		o = strtok_r(NULL, "\'", it);
	else
		o = strtok_r(NULL, " ", it);
	return o;
}

//MinGW doesn't know strtok_r
#ifdef __MINGW32__
char* strtok_r(char* ori, char* tok, char** it)
{
	char token = tok[0];
	
	if(ori != NULL)
		*it = ori;
	if(*it == NULL)
		return NULL;
	while(**it == token)
		(*it)++;
	char* old_it = *it;
	char* c = strchr(*it, token);
	if(c != NULL)
	{
		while(*c == token)
		{
			*c = '\0';
			*it = (++c);
			if(*c == '\0')
				*it = NULL;
		}
	}
	else
		*it = NULL;
	return old_it;
}
#endif
