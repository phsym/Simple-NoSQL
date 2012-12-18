/*
 * concurrency.h
 *
 *  Created on: 11 août 2012
 *      Author: phsymo10
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
