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
 * concurrency.c
 *
 *  Created on: 11 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#include <stdio.h>

#include "concurrency.h"
#include "utils.h"

void thread_create(thread_t* thread, TH_HDL (*handler)(void *), void* args, int detach)
{
#ifdef __MINGW32__
	//TODO: Attach mode
	*thread = _beginthread(handler, 0, args); // Start in detach mode
#else
	if(pthread_create(thread, NULL, handler, args) < 0)
		_perror("Cannot create thread");
	if(detach)
	{
		if(pthread_detach(*thread) < 0)
			_perror("Cannot detach thread");
	}
#endif
}

void thread_join(thread_t* thread)
{
#ifdef __MINGW32__
	WaitForSingleObject((HANDLE)*thread, INFINITE);
	CloseHandle((HANDLE)thread);
#else
	pthread_join(*thread, NULL);
#endif
}


void semaphore_init(semaphore_t* sem, int value, int max_value)
{
#ifdef __MINGW32__
	*sem = CreateSemaphore(NULL, value, max_value, NULL);
#else
	sem_init(sem, 0, value);
#endif
}

void semaphore_destroy(semaphore_t* sem)
{
#ifdef __MINGW32__
	CloseHandle(*sem);
#else
	sem_close(sem);
	sem_destroy(sem);
#endif
}

void semaphore_wait(semaphore_t* sem)
{
#ifdef __MINGW32__
	WaitForSingleObject(*sem, INFINITE);
#else
	sem_wait(sem);
#endif
}

void semaphore_post(semaphore_t* sem)
{
#ifdef __MINGW32__
	ReleaseSemaphore(*sem, 1, NULL);
#else
	sem_post(sem);
#endif
}
