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

static pthread_barrier_t barr;	/* Barrier waited on by all threads */
static int barrs;	/* Number of times the threads will pass the barrier */

static void * barrier(void *);

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
	int i, r, thrs;
	pthread_t *tids;

	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s num-barriers num-threads\n", argv[0]);

	barrs = getint(argv[1]);
	thrs = getint(argv[2]);

	/*
	 * Initialize the barrier. The final argument specifies the number of
	 * threads that must call pthread_barrier_wait() before any thread will
	 * unblock from that call.
	 *
	 * The pthread_barrier_init() function will initialize barrier with
	 * attributes specified in attr, or if it is NULL, with default
	 * attributes. The number of threads that must call
	 * pthread_barrier_wait() before any of the waiting threads can be
	 * released is specified by count. The pthread_barrier_destroy()
	 * function will destroy barrier and release any resources that may
	 * have been allocated on its behalf.
	 */
	if ((r = pthread_barrier_init(&barr, NULL, thrs)) != 0)
		errmsg_exit1("pthread_barrier_init failed, %d\n", r);

	/* Creates 'thrs' threads */
	tids = xmalloc(thrs * sizeof(*tids));
	for (i = 0; i < thrs; i++) {
		r = pthread_create(&tids[i], NULL, barrier, (void *)i);
		if (r != 0)
			errmsg_exit1("pthread_create (%d) failed, %d\n", i, r);
	}

	/*
	 * Each thread prints a start-up message. We briefly delay, and then
	 * print a newline character so that an empty line appears after the
	 * start-up messages.
	 */

	usleep(100000);
	printf("\n");

	/* Wait for all of the threads to terminate */
	for (i = 0; i < thrs; i++)
		if ((r = pthread_join(tids[i], NULL)) != 0)
			errmsg_exit1("pthread_join (%d) failed, %d\n", i, r);

	if ((r = pthread_barrier_destroy(&barr)) != 0)
		errmsg_exit1("pthread_barrier_destroy failed, %d\n", r);
	xfree(tids);

	exit(EXIT_SUCCESS);
}

static void * 
barrier(void *arg)
{
	int thrno, i, r, secs;

	thrno = (int)arg;
	printf("Thread %d started\n", thrno);

	/*
	 * Seed the random number generator based on the current time
	 * (so that we get different seeds on each run) plus thread
	 * number (so that each thread gets a unique seed).
	 */
	srand(time(NULL) + thrno);

	/*
	 * Each thread loops, sleeping for a few seconds and then waiting
	 * on the barrier. The loop terminates when each thread has passed
	 * the barrier 'numBarriers' times.
	 */
	for (i = 0; i < barrs; i++) {
		secs = rand() % 5 + 1;	/* Sleep for 1 to 5 seconds */
		sleep(secs);

		printf("Thread %d about to wait on barrier %d after sleeping "
			"%d seconds\n", thrno, i, secs);

		/*
		 * Calling pthread_barrier_wait() causes each thread to block
		 * until the call has been made by number of threads specified
		 * in the pthread_barrier_init() call.
		 *
		 * The pthread_barrier_wait() function will synchronize calling
		 * threads at barrier. The threads will be blocked from making
		 * further progress until a sufficient number of threads calls
		 * this function. The number of threads that must call it
		 * before any of them will be released is determined by the
		 * count argument to pthread_barrier_init(). Once the threads
		 * have been released the barrier will be reset.
		 *
		 * If the call to pthread_barrier_wait() is successful, all but
		 * one of the threads will return zero. That one thread will
		 * return PTHREAD_BARRIER_SERIAL_THREAD. Otherwise, an error
		 * number will be returned to indicate the error.
		 */
		switch (r = pthread_barrier_wait(&barr)) {
		case 0:
			printf("Thread %d passed barrier %d: return value was "
				"0\n", thrno, i);
			break;
		case PTHREAD_BARRIER_SERIAL_THREAD:
			printf("Thread %d passed barrier %d: return value was "
				"PTHREAD_BARRIER_SERIAL_THREAD\n", thrno, i);

			/*
			 * In the thread that gets the
			 * PTHREAD_BARRIER_SERIAL_THREAD return value, we
			 * briefly delay, and then print a newline character.
			 * This should give all of the threads a chance to print
			 * the message saying they have passed the barrier, and
			 * then provide a newline that separates those messages
			 * from subsequent output. (The only purpose of this
			 * step is to make the program output a little easier
			 * to read.
			 */

			usleep(100000);
			printf("\n");
			break;
		default:
			errmsg_exit1("pthread_barrier_wait failed (%d), %d",
				thrno, r);
		}
	}

	/*
	 * Print out thread termination message after a briefly delaying,
	 * so that the other threads have a chance to display the return
	 * value they received from pthread_barrier_wait(). (This simply
	 * makes the program output a little easier to read.)
	 */

	usleep(200000);
	printf("Thread %d terminating\n", thrno);

	return NULL;
}
