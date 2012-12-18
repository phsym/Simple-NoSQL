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
 * concurrency.h
 *
 *  Created on: 11 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef THREAD_H_
#define THREAD_H_

#ifdef __MINGW32__
	#include <windows.h>
	#include <process.h>
#else
	#include <semaphore.h>
	#include <pthread.h>
#endif

#ifdef __MINGW32__
	#define TH_RETURN return

	typedef uintptr_t thread_t;
	typedef HANDLE semaphore_t;
	typedef void TH_HDL;
#else
	#define TH_RETURN return NULL

	typedef pthread_t thread_t;
	typedef sem_t semaphore_t;
	typedef void* TH_HDL;
#endif

void thread_create(thread_t* thread, TH_HDL (*handler)(void *), void* args, int detach);

void thread_join(thread_t* thread);

/* max_value is for windows only*/
void semaphore_init(semaphore_t* sem, int value, int max_value);

void semaphore_destroy(semaphore_t* sem);

void semaphore_wait(semaphore_t* sem);

void semaphore_post(semaphore_t* sem);

#endif /* THREAD_H_ */
