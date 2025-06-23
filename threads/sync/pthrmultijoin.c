/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025 Jianping Duan <static.integer@hotmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include "unibsd.h"
#include <pthread.h>

static int totthrs = 0;		/* Total number of threads created */
static int numlive = 0;		/* Total number of threads still live or
				terminated but not yet joined */
static int numunjn = 0;		/* Number of terminated threads that have not
				yet been joined */

enum thrstate {
	TS_ALIVE,	/* Thread is alive */
	TS_TERMINATED,	/* Thread terminated, not yet joined */
	TS_JOINED	/* Thread terminated, and joined */
};

static struct {
	pthread_t	mt_tid;		/* ID of this thread */
	enum thrstate	mt_state;	/* Thread state */
	int		mt_slptim;	/* Number of seconds to live */
} *mythread;

static pthread_cond_t	thrcnd = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t	thrmtx = PTHREAD_MUTEX_INITIALIZER;

static void * thread_func(void *);

#ifdef __clang__
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#pragma clang diagnostic ignored "-Wvoid-pointer-to-int-cast"
#elif defined(__GCC__)
#pragma GCC diagnostic ignored "-Wint-to-void-pointer-cast"
#pragma GCC diagnostic ignored "-Wvoid-pointer-to-int-cast"
#endif

int
main(int argc, char *argv[])
{
	int i, r;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s num-secs...\n", argv[0]);

	setbuf(stdout, NULL);

	/* Create all threads */
	mythread = xmalloc((argc - 1) * sizeof(*mythread));
	for (i = 0; i < argc - 1; i++) {
		mythread[i].mt_state = TS_ALIVE;
		mythread[i].mt_slptim = getint(argv[i + 1]);
		r = pthread_create(&mythread[i].mt_tid, NULL, thread_func,
			(void *)i);
		if (r != 0)
			errmsg_exit1("pthread_create failed, %d\n", r);
	}

	totthrs = argc - 1;
	numlive = totthrs;

	/* Join with terminated threads */
	while (numlive > 0) {
		if ((r = pthread_mutex_lock(&thrmtx)) != 0)
			errmsg_exit1("pthread_mutex_lock failed, %d\n", r);

		/*
		 * The pthread_cond_wait() function atomically blocks the
		 * current thread waiting on the condition variable specified by
		 * cond, and releases the mutex specified by mutex. The waiting
		 * thread unblocks only after another thread calls
		 * pthread_cond_signal(3), or pthread_cond_broadcast(3) with the
		 * same condition variable, and the current thread reacquires
		 * the lock on mutex.
		 */
		while (numunjn == 0)
			if ((r = pthread_cond_wait(&thrcnd, &thrmtx)) != 0)
				errmsg_exit1("pthread_cond_wait failed, %d\n",
					r);

		for (i = 0; i < totthrs; i++) {
			if (mythread[i].mt_state == TS_TERMINATED) {
				r = pthread_join(mythread[i].mt_tid, NULL);
				if (r != 0)
					errmsg_exit1("pthread_join failed, "
						"%d\n", r);

				mythread[i].mt_state = TS_JOINED;
				numlive--;
				numunjn--;

				printf("Reaped thread %d (numlive=%d)\n", i,
					numlive);
			}
		}

		if ((r = pthread_mutex_unlock(&thrmtx)) != 0)
			errmsg_exit1("pthread_mutex_unlock failed, %d\n", r);
	}

	exit(EXIT_SUCCESS);
}

static void *
thread_func(void *arg)
{
	int i = (int)arg, r;

	sleep(mythread[i].mt_slptim);	 /* Simulate doing some work */
	printf("Thread %d terminating\n", i);

	if ((r = pthread_mutex_lock(&thrmtx)) != 0)
		errmsg_exit1("pthread_mutex_lock failed, %d\n", r);

	numunjn++;
	mythread[i].mt_state = TS_TERMINATED;

	if ((r = pthread_mutex_unlock(&thrmtx)) != 0)
		errmsg_exit1("pthread_mutex_unlock failed, %d\n", r);
	/*
	 * The pthread_cond_signal() function unblocks one thread waiting for
	 * the condition variable cond.
	 */
	if ((r = pthread_cond_signal(&thrcnd)) != 0)
		errmsg_exit1("pthread_cond_signal failed, %d\n", r);

	return NULL;
}
