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

static pthread_cond_t	cnd = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t	mtx = PTHREAD_MUTEX_INITIALIZER;

static int glob = 0;	/* Predicate variable */

static void cleanup_routine(void *);
static void * thread_func(void *);

int
main(int argc, char *argv[])
{
	pthread_t tid;
	int r;
	void *valptr;

	assert(argv[0] != NULL);

	if ((r = pthread_create(&tid, NULL, thread_func, NULL)) != 0)
		errmsg_exit1("pthread_create failed, %d\n", r);

	sleep(2);	/* Give thread a chance to get started */

	if (argc == 1) {
		printf("Main:\t\tabout to cancel\n");
		if ((r = pthread_cancel(tid)) != 0)
			errmsg_exit1("pthread_cancel failed, %d\n", r);
		goto nothrcnd;
	}

	printf("Main:\t\tabout to condition variable\n");
	
	if ((r = pthread_mutex_lock(&mtx)) != 0)
		errmsg_exit1("pthread_mutex_lock failed, %d\n", r);

	glob = 1;

	if ((r = pthread_mutex_unlock(&mtx)) != 0)
		errmsg_exit1("pthread_mutex_unlock failed, %d\n", r);

	if ((r = pthread_cond_signal(&cnd)) != 0)
		errmsg_exit1("pthread_cond_signal failed, %d\n", r);

nothrcnd:
	if ((r = pthread_join(tid, &valptr)) != 0)
		errmsg_exit1("pthread_join failed, %d\n", r);
	
	if (valptr == PTHREAD_CANCELED)
		printf("main:\t\tthread waw canceled\n");
	else
		printf("main:\t\tthread terminated normally\n");

	exit(EXIT_SUCCESS);
}

static void *
thread_func(void *arg)
{
	int r;
	char *buf;

	assert(arg == NULL);

	buf = xmalloc(1024);
	printf("Thread:\t\tallocate memory at %p\n", buf);

	/* Not a cancellation point */
	if ((r = pthread_mutex_lock(&mtx)) != 0)
		errmsg_exit1("pthread_mutex_lock failed, %d\n", r);

	/*
	 * The pthread_cleanup_push() function adds cleanup_routine to the top
	 * of the stack of cleanup handlers that get called when the current
	 * thread exits.
	 *
	 * The pthread_cleanup_push() function is implemented as a macro that
	 * opens a new block. Invocations of this function must appear as
	 * standalone statements that are paired with a later call of
	 * pthread_cleanup_pop(3) in the same lexical scope.
	 */
	pthread_cleanup_push(cleanup_routine, buf);

	while (glob == 0) {
		/* A cancellation point */
		r = pthread_cond_wait(&cnd, &mtx);
		if (r != 0)
			errmsg_exit1("pthread_cond_wait failed, %d\n", r);
	}

	printf("Thread:\t\tcondition wait loop completed\n");
	/*
	 * The pthread_cleanup_pop() function pops the top cleanup routine off
	 * of the current threads cleanup routine stack, and, if execute is
	 * non-zero, it will execute the function. If there is no cleanup
	 * routine then pthread_cleanup_pop() does nothing.
	 */
	pthread_cleanup_pop(1);

	return NULL;
}

static void
cleanup_routine(void *arg)
{
	int r;

	printf("Cleanup:\tfreeing block at %p\n", arg);
	xfree(arg);

	printf("Cleanup:\tunlock mutex\n");
	if ((r = pthread_mutex_unlock(&mtx)) != 0)
		errmsg_exit1("pthread_mutex_unlock failed, %d\n", r);
}
