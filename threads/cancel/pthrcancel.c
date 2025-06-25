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

static void * thread_func(void *);

int
main(void)
{
	pthread_t tid;
	int r;
	void *valptr;

	if ((r = pthread_create(&tid, NULL, thread_func, NULL)) != 0)
		errmsg_exit1("pthread_create failed, %d\n", r);

	sleep(5);	/* Allow new thread to run a while */

	/*
	 * The pthread_cancel() function requests that thread be canceled. The
	 * target thread's cancelability state and type determines when the
	 * cancellation takes effect. When the cancellation is acted on, the
	 * cancellation cleanup handlers for thread are called. When the last
	 * cancellation cleanup handler returns, the thread-specific data
	 * destructor functions will be called for thread. When the last
	 * destructor function returns, thread will be terminated.
	 *
	 * The cancellation processing in the target thread runs asynchronously
	 * with respect to the calling thread returning from pthread_cancel().
	 *
	 * A status of PTHREAD_CANCELED is made available to any threads joining
	 * with the target. The symbolic constant PTHREAD_CANCELED expands to a
	 * constant expression of type (void *), whose value matches no pointer
	 * to an object in memory nor the value NULL
	 */
	if ((r = pthread_cancel(tid)) != 0)
		errmsg_exit1("pthread_cancel failed, %d\n", r);

	if ((r = pthread_join(tid, &valptr)) != 0)
		errmsg_exit1("pthread_join failed, %d\n", r);

	if (valptr == PTHREAD_CANCELED)
		printf("Thread was canceled\n");
	else
		printf("Thread was not canceled (should not happen!)\n");

	exit(EXIT_SUCCESS);
}

static void *
thread_func(void *arg)
{
	int i;

	assert(arg == NULL);
	printf("New thread started\n");	/* May be a cancellation point */

	for (i = 1; ; i++) {
		printf("Loop %d\n", i);	/* May be a cancellation point */
		sleep(1);		/* A cancellation point */
	}

	return NULL;
}
