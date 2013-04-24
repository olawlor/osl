/**
 * PorThread:
 *  Portable, trivial threading library.
 * Orion Sky Lawlor, olawlor@acm.org, 2003/4/2
 */
#include <stdio.h>
#include <stdlib.h>
#include "porthread.h"  /* osl/porthread.h */

struct porthread_rec {
	porthread_fn_t fn;
	void *arg;
	porthread_rec(porthread_fn_t fn_,void *arg_)
		:fn(fn_), arg(arg_) {}
};

/**
 * This routine is called to start the new thread:
 * The argument should be dynamically allocated with "new".
 */
inline void porthread_run(porthread_rec *r) {
	porthread_fn_t fn=r->fn;
	void *arg=r->arg;
	delete r;
	(fn)(arg);
}


#ifdef _WIN32
/****** System Specifics: Win32 thread *******/
#include <windows.h>

DWORD WINAPI porthread_start_shim(LPVOID param)
{
	porthread_run((porthread_rec *)param);
	return 0;
}

/**
 * Call fn(arg) from within a new thread.
 */
porthread_t porthread_create(porthread_fn_t fn,void *arg) {
	HANDLE  thr;
	DWORD   threadID;
	thr = CreateThread(NULL, 0, porthread_start_shim, 
		(LPVOID)new porthread_rec(fn,arg), 0, &threadID);
	return (porthread_t)thr;
}

/** Wait until this thread has finished running. */
void porthread_wait(porthread_t p)
{
	WaitForSingleObject((HANDLE)p,INFINITE);
	CloseHandle((HANDLE)p);
}
void porthread_detach(porthread_t p)
{
	CloseHandle((HANDLE)p);
}

void porthread_yield(int msec) {
	Sleep(msec);
}

#else 
/******* System Specifics: (non-windows) POSIX thread *******/
#include <pthread.h>
#include <unistd.h> /* for usleep */

void pthreadErrCheck(int errCode) {
	if (errCode!=0) {
		printf("Error %d creating pthread!\n",errCode);
		exit(1);
	}
}

extern "C" 
void *porthread_start_shim(void *param) {
	porthread_run((porthread_rec *)param);
	return 0;
}


/**
 * Call fn(arg) from within a new thread.
 */
porthread_t porthread_create(porthread_fn_t fn,void *arg) {
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthreadErrCheck(pthread_create(&pid, &attr, porthread_start_shim, 
			(void *)new porthread_rec(fn,arg)));
	return (porthread_t)pid;
}

/** Wait until this thread has finished running. */
void porthread_wait(porthread_t p)
{
	pthread_join((pthread_t)p,0);
}
void porthread_detach(porthread_t p)
{
	pthread_detach((pthread_t)p);
}

void porthread_yield(int msec) {
	usleep(msec*1000);
}


#endif
