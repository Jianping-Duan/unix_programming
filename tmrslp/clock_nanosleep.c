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
#include <time.h>
#include <sys/time.h>
#include <signal.h>

static void int_handler(int);

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	struct timespec req, rem;
	struct timeval start, end;
	int flags, s;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s secs nsecs [a]\n", argv[0]);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = int_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	flags = (argc > 3) ? TIMER_ABSTIME : 0;

	if (flags == TIMER_ABSTIME) {
		if (clock_gettime(CLOCK_REALTIME, &req) == -1)
			errmsg_exit1("clock_gettime failed, %s\n", ERR_MSG);
		printf("Initial CLOCK_REALTIME value: %ld.%09ld\n",
			req.tv_sec, req.tv_nsec);

		req.tv_sec += getlong(argv[1], 0);
		req.tv_nsec += getlong(argv[1], 0);
	} else {	/* Relative sleep */
		req.tv_sec = getlong(argv[1], 0);
		req.tv_nsec = getlong(argv[2], 0);
	}

	if (req.tv_nsec >= 1000000000) {	/* In-position conversion */
			req.tv_sec += req.tv_nsec / 1000000000;
			req.tv_nsec %= 1000000000;
	}

	if (gettimeofday(&start, NULL) == -1)
		errmsg_exit1("gettimeofday failed, %s\n", ERR_MSG);

	while (1) {
		/*
		 * If the TIMER_ABSTIME flag is not set in the flags argument,
		 * then clock_nanosleep() suspends execution of the calling
		 * thread until either the time interval specified by the rqtp
		 * argument has elapsed, or a signal is delivered to the calling
		 * process and its action is to invoke a signal-catching
		 * function or to terminate the process. The clock used to
		 * measure the time is specified by the clock_id argument.
		 *
		 * If the TIMER_ABSTIME flag is set in the flags argument,
		 * then clock_nanosleep() suspends execution of the calling
		 * thread until either the value of the clock specified by
		 * the clock_id argument reaches the absolute time specified by
		 * the rqtp argument, or a signal is delivered to the calling
		 * process and its action is to invoke a signal-catching
		 * function or to terminate the process. If, at the time of the
		 * call, the time value specified by rqtp is less than or equal
		 * to the time value of the specified clock, then
		 * clock_nanosleep() returns immediately and the calling thread
		 * is not suspended.
		 *
		 * If these functions return for any other reason, then
		 * clock_nanosleep() will directly return the error number. If a
		 * relative sleep is interrupted by a signal and rmtp is
		 * non-NULL, the timespec structure it references is updated to
		 * contain the unslept amount (the request time minus the time
		 * actually slept).
		 */
		s = clock_nanosleep(CLOCK_REALTIME, flags, &req, &rem);
		if (s != 0 && s != EINTR)
			errmsg_exit1("clock_nanosleep failed, %s\n",
				strerror(s));

		if (s == EINTR)
			fprintf(stderr, "Interrupted...\n");

		if (gettimeofday(&end, NULL) == -1)
			errmsg_exit1("gettimeofday failed, %s\n", ERR_MSG);
		printf("Shept: %.6f secs", end.tv_sec - start.tv_sec +
			(end.tv_usec - start.tv_usec) / 1000000.0);

		if (s == 0)
			break;	/* sleep completed */

		if (flags != TIMER_ABSTIME) {
			printf("...Remaining: %ld.%09ld", rem.tv_sec,
				rem.tv_nsec);
			req = rem;
		}

		printf("...Restarting\n");
	}

	printf("\nSleep complete\n");
	
	exit(EXIT_SUCCESS);
}

static void
int_handler(int sig)
{
	printf("\tCaught %d (%s)\n", sig, strsignal(sig));
}
