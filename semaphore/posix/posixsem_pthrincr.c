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
#include <semaphore.h>
#include <pthread.h>

static volatile int glob = 0;
static sem_t sem;

static void * thread_func(void *);

int
main(int argc, char *argv[])
{
	pthread_t *tids;
	int i, r, thrs, loops;

	if (argc == 2 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [threads] [loops]\n", argv[0]);

	thrs = (argc >= 2) ? getint(argv[1]) : 2;
	loops = (argc >= 3) ? getint(argv[2]) : 100000;
	assert(thrs >= 2 && loops > 0);

	/* Initialize a semaphore with the value 1 */

	/*
	 * The sem_init() function initializes the unnamed semaphore pointed to
	 * by sem to have the value value.
	 *
	 * A non-zero value for pshared specifies a shared semaphore that can be
	 * used by multiple processes, the semaphore should be located in shared
	 * memory region (see mmap(2), shm_open(2), and shmget(2)), any process
	 * having read and write access to address sem can perform semaphore
	 * operations on sem.
	 *
	 * If pshared is equal to zero, the semaphore will be shared among
	 * threads within the calling process. In this case, sem is typically
	 * specified as the address of a global variable or the address of a
	 * variable allocated on the heap. Semaphores shared by threads have
	 * process persistence, and they will be destroyed when the process
	 * terminates
	 */
	if (sem_init(&sem, 0, 1) == -1)
		errmsg_exit1("sem_init (0, 1) failed, %s\n", ERR_MSG);

	/* Creates some pthreads that increment 'glob' */

	tids = xmalloc(sizeof(*tids) * thrs);

	for (i = 0; i < thrs; i++) {
		r = pthread_create(&tids[i], NULL, thread_func, &loops);
		if (r == -1)
			errmsg_exit1("pthread_create (%d) failed, %d)", i, r);
	}

	/* Wait for threads to terminate */

	for (i = 0; i < thrs; i++)
		if ((r = pthread_join(tids[i], NULL)) == -1)
			errmsg_exit1("pthread_join (%d) failed, %d\n", i, r);

	printf("glob = %d\n", glob);

	if (sem_destroy(&sem) == -1)
		errmsg_exit1("sem_destroy failed, %s\n", ERR_MSG);
	xfree(tids);

	exit(EXIT_SUCCESS);
}

static void *
thread_func(void *arg)
{
	int loops = *((int *)arg), i, loc;

	for (i = 0; i < loops; i++) {
		if (sem_wait(&sem) == -1)
			errmsg_exit1("sem_wait failed, %s\n", ERR_MSG);

		loc = glob;
		loc++;
		glob = loc;

		if (sem_post(&sem) == -1)
			errmsg_exit1("sem_post failed, %s\n", ERR_MSG);
	}

	return NULL;
}
