/*
 * concurrency.c
 *
 *  Created on: 11 août 2012
 *      Author: phsymo10
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
