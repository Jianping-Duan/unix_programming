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
	struct timeval start, end;
	struct timespec req, rem;
	int s;

	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s secs nanosecs\n", argv[0]);

	req.tv_sec = getlong(argv[1], 0);
	req.tv_nsec = getlong(argv[2], 0);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = int_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	if (gettimeofday(&start, NULL) == -1)
		errmsg_exit1("gettimeofday (1) failed, %s\n", ERR_MSG);

	for (;;) {
		/*
		 * The nanosleep suspends execution of the calling thread until
		 * either the time interval specified by the rqtp argument has
		 * elapsed, or a signal is delivered to the calling process and
		 * its action is to invoke a signal-catching function or to
		 * terminate the process. 
		 *
		 * The suspension time may be longer than requested due to the
		 * scheduling of other activity by the system. It is also
		 * subject to the allowed time interval deviation specified by
		 * the kern.timecounter.alloweddeviation sysctl(8) variable. An
		 * unmasked signal will terminate the sleep early, regardless of
		 * the SA_RESTART value on the interrupting signal. The rqtp and
		 * rmtp arguments can point to the same object.
		 */
		if ((s = nanosleep(&req, &rem)) == -1 && errno != EINTR)
			errmsg_exit1("nanosleep failed, %s\n", ERR_MSG);

		if (gettimeofday(&end, NULL) == -1)
			errmsg_exit1("gettimeofday (2) failed, %s\n", ERR_MSG);
		printf("Slept for: %9.2f secs\n", end.tv_sec - start.tv_sec + 
			(end.tv_usec - start.tv_usec) / 1000000.0);

		if (s == 0)
			break;		/* nanosleep() completed */

		printf("Remaining: %2ld.%09ld\n", rem.tv_sec, rem.tv_nsec);
		req = rem;		/* Next sleep is with remaining time */
	}

	exit(EXIT_SUCCESS);
}

static void
int_handler(int sig)
{
	printf("\tCaught %d (%s)\n", sig, strsignal(sig));
	return;
}
