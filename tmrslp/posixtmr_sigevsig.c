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
#include <signal.h>
#include "currtime.h"
#include "strtoitrspec.h"

static void rtmax_handler(int, siginfo_t *, void *);

int
main(int argc, char *argv[])
{
	timer_t *trlst;
	struct sigaction sa;
	struct sigevent sev;
	struct itimerspec ts;
	int i;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s secs[/nsecs][:int-secs[/int-nsecs]]..."
			"\n", argv[0]);

	trlst = xcalloc(argc - 1, sizeof(*trlst));

	/* Establish handler for notification signal */

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = rtmax_handler;
	if (sigaction(SIGRTMAX, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	/* Create and start one timer for each command-line argument */

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMAX;

	for (i = 0; i < argc - 1; i++) {
		strtoitrspec(argv[i + 1], &ts);

		/* Allows handler to get ID of this timer */
		sev.sigev_value.sigval_ptr = &trlst[i];

		/*
		 * The timer_create() system call creates a per-process timer
		 * using the specified clock, clock_id, as the timing base. The
		 * timer_create() system call returns, in the location
		 * referenced by timerid, a timer ID of type timer_t used to
		 * identify the timer in timer requests. This timer ID is unique
		 * within the calling process until the timer is deleted. The
		 * particular clock, clock_id, is defined in <time.h>. The timer
		 * whose ID is returned is in a disarmed state upon return from
		 * timer_create().
		 *
		 * The evp argument, if non-NULL, points to a sigevent
		 * structure. This structure, allocated by the application,
		 * defines the asynchronous notification to occur when the timer
		 * expires.
		 *
		 * If evp->sigev_notify is SIGEV_SIGNO or SIGEV_THREAD_ID, the
		 * signal specified in evp->sigev_signo will be sent to the
		 * calling process (SIGEV_SIGNO) or to the thread whose LWP ID
		 * is evp->sigev_notify_thread_id (SIGEV_THREAD_ID). The
		 * information for the queued signal will include:
		 *
		 * Member		Value
		 * si_code		SI_TIMER
		 * si_value		the value stored in evp->sigev_value
		 * si_timerid		timer ID
		 * si_overrun		timer overrun count
		 * si_errno		If timer overrun is {DELAYTIMER_MAX}, an
		 *			error code defined in <errno.h>
		 *
		 * If the evp argument is NULL, the effect is as if the evp
		 * argument pointed to a sigevent structure with the
		 * sigev_notify member having the value SIGEV_SIGNAL, the
		 * sigev_signo having a default signal number (SIGALRM), and
		 * the sigev_value member having the value of the timer ID.
		 *
		 * This implementation supports a clock_id of CLOCK_REALTIME or
		 * CLOCK_MONOTONIC.
		 * 
		 */
		if (timer_create(CLOCK_REALTIME, &sev, &trlst[i]) == -1)
			errmsg_exit1("timer_create failed, %s\n", ERR_MSG);

		/*
		 * The timer_settime() system call sets the time until the next
		 * expiration of the timer specified by timerid from the
		 * it_value member of the value argument and arms the timer if
		 * the it_value member of value is non-zero. If the specified
		 * timer was already armed when timer_settime() is called, this
		 * call resets the time until next expiration to the value
		 * specified. If the it_value member of value is zero, the timer
		 * is disarmed. If the timer is disarmed, then pending signal is
		 * removed.
		 */
		if (timer_settime(trlst[i], 0, &ts, NULL) == -1)
			errmsg_exit1("timer_settime failed, %s\n", ERR_MSG);
	}

	while (1)	/* Wait for incoming timer signals */
		pause();

	xfree(trlst);

	exit(EXIT_SUCCESS);
}

static void
rtmax_handler(int sig, siginfo_t *si, void *uc)
{
	timer_t *tid;

	tid = si->si_value.sigval_ptr;

	printf("[%s] Got signal %d\n", currtime("%T"), sig);
	/*
	 * Only a single signal is queued to the process for a given timer at
	 * any point in time. When a timer for which a signal is still pending
	 * expires, no signal is queued, and a timer overrun will occur. When a
	 * timer expiration signal is accepted by a process, the
	 * timer_getoverrun() system call returns the timer expiration overrun
	 * count for the specified timer. The overrun count returned contains
	 * the number of extra timer expirations that occurred between the time
	 * the signal was generated (queued) and when it was accepted, up to but
	 * not including an maximum of {DELAYTIMER_MAX}. If the number of such
	 * extra expirations is greater than or equal to {DELAYTIMER_MAX}, then
	 * the overrun count is set to {DELAYTIMER_MAX}. The value returned by
	 * timer_getoverrun() applies to the most recent expiration signal
	 * acceptance for the timer. If no expiration signal has been delivered
	 * for the timer, the return value of timer_getoverrun() is unspecified.
	 *
	 * If the timer_getoverrun() system call succeeds, it returns the timer
	 * expiration overrun count as explained above. Otherwise the value -1
	 * is returned, and the global variable errno is set to indicate the
	 * error.
	 */
	printf("\ttimer_getoverrun = %d\n", timer_getoverrun(*tid));
}
