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

static volatile sig_atomic_t got_alarm = 0;

static void display_timers(const char *, bool);
static void alarm_handler(int);

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	struct itimerval itv;
	clock_t prevclk;
	int maxsigs, sigcnt;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [secs [usec [int-secs [int-usecs]]]]"
			"\n", argv[0]);

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = alarm_handler;
	sa.sa_flags = 0;
	if (sigaction(SIGALRM, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	itv.it_value.tv_sec = (argc > 1) ? getlong(argv[1], 0) : 5;
	itv.it_value.tv_usec = (argc > 2) ? getlong(argv[2], 0) : 0;
	itv.it_interval.tv_sec = (argc > 3) ? getlong(argv[3], 0) : 0;
	itv.it_interval.tv_usec = (argc > 4) ? getlong(argv[4], 0) : 0;

	/* exit after 3 signals, or on first signal if interval is 0 */
	maxsigs = (itv.it_interval.tv_sec == 0 &&
		itv.it_interval.tv_usec == 0) ? 1 : 3;

	display_timers("START:", false);
	/*
	 * The system provides each process with three interval timers, defined
	 * in <sys/time.h>. The setitimer() system call sets a timer to the
	 * specified value (returning the previous value of the timer if ovalue
	 * is not a null pointer)
	 *
	 * A timer value is defined by the itimerval structure.
	 *
	 * If it_value is non-zero, it indicates the time to the next timer
	 * expiration. If it_interval is non-zero, it specifies a value to be
	 * used in reloading it_value when the timer expires. Setting it_value
	 * to 0 disables a timer, regardless of the value of it_interval.
	 * Setting it_interval to 0 causes a timer to be disabled after its next
	 * expiration (assuming it_value is non-zero).
	 *
	 * The ITIMER_REAL timer decrements in real time. A SIGALRM signal is
	 * delivered when this timer expires.
	 *
	 * The ITIMER_VIRTUAL timer decrements in process virtual time. It runs
	 * only when the process is executing. A SIGVTALRM signal is delivered
	 * when it expires.
	 *
	 * The ITIMER_PROF timer decrements both in process virtual time and
	 * when the system is running on behalf of the process. It is designed
	 * to be used by interpreters in statistically profiling the execution
	 * of interpreted programs. Each time the ITIMER_PROF timer expires, the
	 * SIGPROF signal is delivered. Because this signal may interrupt in-
	 * progress system calls, programs using this timer must be prepared to
	 * restart interrupted system calls.
	 */
	if (setitimer(ITIMER_REAL, &itv, NULL) == -1)
		errmsg_exit1("setitimer failed, %s\n", ERR_MSG);

	prevclk = clock();
	sigcnt = 0;

	for (;;) {
		/* Inner loop consumes at least 0.5 seconds CPU time */
		while (((clock() - prevclk) * 10 / CLOCKS_PER_SEC) < 5) {
			if (!got_alarm)
				continue;

			got_alarm = 0;
			display_timers("ALARM:", true);

			if (++sigcnt >= maxsigs) {
				printf("That's all folks\n");
				exit(EXIT_SUCCESS);
			}
		}

		prevclk = clock();
		display_timers("Main:", true);
	}

	exit(EXIT_SUCCESS);
}

static void
alarm_handler(int sig)
{
	assert(sig == SIGALRM);
	printf("\tCaught %d (%s)\n", sig, strsignal(sig));
	got_alarm = 1;
}

static void
display_timers(const char *msg, bool incldtmr)
{
	static int call_num = 0;	/* Number of calls to this function */
	static struct timeval start;
	struct timeval curr;
	struct itimerval itv;

	if (call_num == 0)	/* Initialize elapsed time meter */
		if (gettimeofday(&start, NULL) == -1)
			errmsg_exit1("gettimeofday failed, %s\n", ERR_MSG);

	if (call_num % 20 == 0)
		printf("\tElapsed    Value Interval\n");

	if (gettimeofday(&curr, NULL) == -1)
		errmsg_exit1("gettimeofday failed, %s\n", ERR_MSG);
	printf("%-7s %6.2f", msg, curr.tv_sec - start.tv_sec +
		(curr.tv_usec - start.tv_usec) / 1000000.0);

	if (!incldtmr)
		goto no_incld_tmr;

	/*
	 * The getitimer() system call returns the current value for the timer
	 * specified in which in the structure at value.
	 */
	if (getitimer(ITIMER_REAL, &itv) == -1)
		errmsg_exit1("getitimer failed, %s\n", ERR_MSG);
	printf("  %6.2f  %6.2f",
		itv.it_value.tv_sec + itv.it_value.tv_usec / 1000000.0,
		itv.it_interval.tv_sec + itv.it_interval.tv_usec / 1000000.0);

no_incld_tmr:
	
	printf("\n");
	call_num++;
}
