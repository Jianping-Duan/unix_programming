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
#include <time.h>

static pthread_mutex_t	mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t	cnd = PTHREAD_COND_INITIALIZER;

static int avail = 0;

#define MAX_THREADS	16

static void * producter(void *);

int
main(int argc, char *argv[])
{
	int i, r;
	int totreqs = 0, numcons = 0;
	bool done = false;
	pthread_t tids[MAX_THREADS];
	time_t t = time(NULL);

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s number...\n", argv[0]);
	if (argc - 1 > MAX_THREADS)
		errmsg_exit1("Too many arguments, %d\n", argc - 1);

	/* Creates all threads */
	for (i = 0; i < argc - 1; i++) {
		r = getint(argv[i + 1]);
		assert(r > 0);
		totreqs += r;

		r = pthread_create(&tids[i], NULL, producter, argv[i + 1]);
		if (r != 0)
			errmsg_exit1("pthread_create failed, %d\n", r);
	}

	/* Loop to consume available units */
	while (1) {
		if ((r = pthread_mutex_lock(&mtx)) != 0)
			errmsg_exit1("pthread_mutex_lock failed, %d\n", r);

		while (avail == 0)	/* Wait for something to consume */
			if ((r = pthread_cond_wait(&cnd, &mtx)) != 0)
				errmsg_exit1("pthread_cond_wait failed, %d\n",
					r);

		/* At this point, 'mtx' is locked... */

		while (avail > 0) {	/* Consume all available units */
			/* Do something with produced unit */
			numcons++;
			avail--;
			printf("TS=%ld: consumed = %d\n",
				(long)(time(NULL) - t), numcons);
			done = numcons >= totreqs;
		}

		if ((r = pthread_mutex_unlock(&mtx)) != 0)
			errmsg_exit1("pthread_mutex_unlock failed, %d\n", r);

		if (done)
			break;

		/* Perhaps do other work here that does not require mutex lock */
	}

	exit(EXIT_SUCCESS);
}

static void *
producter(void *arg)
{
	int i, r, cnt;

	cnt = getint((char *)arg);
	for (i = 0; i < cnt; i++) {
		sleep(1);

		/* Code to produce a unit omitted */

		if ((r = pthread_mutex_lock(&mtx)) != 0)
			errmsg_exit1("pthread_mutex_lock failed, %d\n", r);

		/* Let consumer know another unit is available */
		avail++;

		if ((r = pthread_mutex_unlock(&mtx)) != 0)
			errmsg_exit1("pthread_mutex_unlock failed, %d\n", r);

		/* Wake sleeping consumer */
		if ((r = pthread_cond_signal(&cnd)) != 0)
			errmsg_exit1("pthread_cond_signal failed, %d\n", r);
	}

	return NULL;
}
