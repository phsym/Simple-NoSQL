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
	UNUSED(max_value); // max_value is only for MinGW
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


void mutex_init(mutex_t* mut)
{
#ifdef __MINGW32__
	*mut = CreateMutex(NULL, false, NULL);
#else
	pthread_mutex_init(mut, NULL);
#endif
}

void mutex_destroy(mutex_t* mut)
{
#ifdef __MINGW32__
	CloseHandle(*mut);
#else
	pthread_mutex_destroy(mut);
#endif
}

void mutex_lock(mutex_t* mut)
{
#ifdef __MINGW32__
	WaitForSingleObject(*mut, INFINITE);
#else
	pthread_mutex_lock(mut);
#endif
}

void mutex_unlock(mutex_t* mut)
{
#ifdef __MINGW32__
	ReleaseMutex(*mut);
#else
	pthread_mutex_unlock(mut);
#endif
}

/*
 * Read-Write lock implementations for win32 API comes from Jordan Zimmerman
 * on http://groups.google.com/group/comp.programming.threads/msg/b831174c04245657?hl=en
 */

void rw_lock_init(rw_lock_t* lock)
{
#ifdef __MINGW32__
	InitializeCriticalSection(&lock->lock);
	InitializeCriticalSection(&lock->readlock);

	lock->readers = 0;
	lock->writelock = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	pthread_rwlock_init(lock, NULL);
#endif
}

void rw_lock_destroy(rw_lock_t* lock)
{
#ifdef __MINGW32__
	CloseHandle(lock->writelock);
	DeleteCriticalSection(&lock->lock);
	DeleteCriticalSection(&lock->readlock);
#else
	pthread_rwlock_destroy(lock);
#endif
}

void rw_lock_read_lock(rw_lock_t* lock)
{
#ifdef __MINGW32__
	EnterCriticalSection(&lock->readlock);

	EnterCriticalSection(&lock->lock);
	lock->readers++;
	ResetEvent(lock->writelock);
	LeaveCriticalSection(&lock->lock);

	LeaveCriticalSection(&lock->readlock);
#else
	pthread_rwlock_rdlock(lock);
#endif
}

void rw_lock_read_unlock(rw_lock_t* lock)
{
#ifdef __MINGW32__
	EnterCriticalSection(&lock->lock);
	if (--lock->readers == 0)
		SetEvent(lock->writelock);
	LeaveCriticalSection(&lock->lock);
#else
	pthread_rwlock_unlock(lock);
#endif
}

void rw_lock_write_lock(rw_lock_t* lock)
{
#ifdef __MINGW32__
	EnterCriticalSection(&lock->readlock);

	top: EnterCriticalSection(&lock->lock);
	if (lock->readers) {
		LeaveCriticalSection(&lock->lock);
		WaitForSingleObject(lock->writelock, INFINITE);
		goto top;
	}

	LeaveCriticalSection(&lock->readlock);
#else
	pthread_rwlock_wrlock(lock);
#endif
}

void rw_lock_write_unlock(rw_lock_t* lock)
{
#ifdef __MINGW32__
	LeaveCriticalSection(&lock->lock);
#else
	pthread_rwlock_unlock(lock);
#endif
}
