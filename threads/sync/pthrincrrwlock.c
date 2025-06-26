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
static pthread_rwlock_t rwlck;

#define MAX_THREADS	4

static void * thread_func(void *);

int
main(int argc, char *argv[])
{
	int loops, i, r;
	pthread_t tids[MAX_THREADS];

	loops = (argc > 1 ) ? getint(argv[1]) : 1000000;

	/*
	 * The pthread_rwlock_init() function is used to initialize a read/write
	 * lock, with attributes specified by attr. If attr is NULL, the default
	 * read/write lock attributes are used.
	 */
	if ((r = pthread_rwlock_init(&rwlck, NULL)) != 0)
		errmsg_exit1("pthread_rwlock_init failed, %d\n", r);

	for (i = 0; i < MAX_THREADS; i++) {
		r = pthread_create(&tids[i], NULL, thread_func, (void *)&loops);
		if (r != 0)
			errmsg_exit1("pthread_create (%d) failed, %d\n", i, r);
	}

	for (i = 0; i < MAX_THREADS; i++)
		if ((r = pthread_join(tids[i], NULL)) != 0)
			errmsg_exit1("pthread_join (%d) failed, %d\n", i, r);

	printf("glob = %d\n", glob);
	exit(EXIT_SUCCESS);
}

static void *
thread_func(void *arg)
{
	int loops, i, r;
	int loc = 0;

	loops = *((int *)arg);
	for (i = 0; i < loops; i++) {
		/*
		 * The pthread_rwlock_wrlock() function blocks until a write
		 * lock can be acquired against lock.
		 * The pthread_rwlock_trywrlock() function performs the same
		 * action, but does not block if the lock cannot be immediately
		 * obtained.
		 */
		if ((r = pthread_rwlock_wrlock(&rwlck)) != 0)
			errmsg_exit1("pthread_rwlock_wrlock failed, %d\n", r);

		loc = glob;
		loc++;
		glob = loc;

		/*
		 * The pthread_rwlock_unlock() function is used to release the
		 * read/write lock previously obtained by
		 * pthread_rwlock_rdlock(), pthread_rwlock_wrlock(),
		 * pthread_rwlock_tryrdlock(), or pthread_rwlock_trywrlock().
		 */
		if ((r = pthread_rwlock_unlock(&rwlck)) != 0)
			errmsg_exit1("pthread_rwlock_unlock failed, %d\n", r);
	}

	return NULL;
}
