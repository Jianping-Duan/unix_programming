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

static volatile int glob = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void * thread_func(void *);

int
main(int argc, char *argv[])
{
	int loops, rv;
	pthread_t pt1, pt2;

	loops = (argc > 1 ) ? getint(argv[1]) : 1000000;

	rv = pthread_create(&pt1, NULL, thread_func, (void *)&loops);
	if (rv != 0)
		errmsg_exit1("pthread_create (1) failed, %s\n", ERR_MSG);

	rv = pthread_create(&pt2, NULL, thread_func, (void *)&loops);
	if (rv != 0)
		errmsg_exit1("pthread_create (2) failed, %s\n", ERR_MSG);

	if (pthread_join(pt1, NULL) != 0)
		errmsg_exit1("pthread_join (1) failed, %s\n", ERR_MSG);

	if (pthread_join(pt2, NULL) != 0)
		errmsg_exit1("pthread_join (2) failed, %s\n", ERR_MSG);

	printf("glob = %d\n", glob);
	exit(EXIT_SUCCESS);
}

static void *
thread_func(void *arg)
{
	int loops, i, loc = 0;

	loops = *((int *)arg);
	for (i = 0; i < loops; i++) {
		/*
		 * The pthread_mutex_lock() function locks mutex. If the mutex
		 * is already locked, the calling thread will block until the
		 * mutex becomes available.
		 */
		if (pthread_mutex_lock(&mtx) != 0)
			errmsg_exit1("pthread_mutex_lock failed, %s\n", ERR_MSG);

		loc = glob;
		loc++;
		glob = loc;

		/*
		 * If the current thread holds the lock on mutex, then the
		 * pthread_mutex_unlock() function unlocks mutex. If the
		 * argument pointed by the mutex is a robust mutex in the
		 * inconsistent state, and the call to
		 * pthread_mutex_consistent() function was not done prior to
		 * unlocking, further locking attempts on the mutex mutex are
		 * denied and locking functions return ENOTRECOVERABLE error.
		 */
		if (pthread_mutex_unlock(&mtx) != 0)
			errmsg_exit1("pthread_mutex_unlock failed, %s\n", ERR_MSG);
	}

	return NULL;
}
