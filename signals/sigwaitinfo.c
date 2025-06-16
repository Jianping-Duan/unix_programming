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
#include <signal.h>

int
main(int argc, char *argv[])
{
	int sig, delay;
	siginfo_t si;
	sigset_t allsigs;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [delay-secs]\n", argv[0]);

	printf("%s: PID is %d\n", argv[0], getpid());

	/* Block all signals (except SIGKILL and SIGSTOP) */
	sigfillset(&allsigs);
	if (sigprocmask(SIG_SETMASK, &allsigs, NULL) == -1)
		errmsg_exit1("sigprocmask - SIG_SETMASK failed, %s\n", ERR_MSG);
	printf("%s: signals blocked.\n", argv[0]);
	
	/* Delay so that signals can be sent to us */
	if (argc > 1) {
		delay = getint(argv[1]);
		printf("%s: abort delay %d seconds.\n", argv[0], delay);
		sleep(delay);
		printf("%s: finished delay.\n", argv[0]);
	}

	/* 
	 * Fetch signals until SIGINT (Control-C), SIGQUIT (Control-\) or
	 * SIGTERM
	 */
	while (1) {
		/*
		 * The sigwaitinfo() system call selects the pending signal
		 * from the set specified by set. Should any of multiple pending
		 * signals in the range SIGRTMIN to SIGRTMAX be selected, it
		 * shall be the lowest numbered one. The selection order between
		 * realtime and non-realtime signals, or between multiple
		 * pending non-realtime signals, is unspecified. If no signal in
		 * set is pending at the time of the call, the calling thread is
		 * suspended until one or more signals in set become pending or
		 * until it is interrupted by an unblocked, caught signal.
		 *
		 * Upon successful completion (that is, one of the signals
		 * specified by set is pending or is generated) sigwaitinfo()
		 * return the selected signal number. Otherwise, the functions
		 * return a value of -1 and set the global variable errno to
		 * indicate the error.
		 */
		if ((sig = sigwaitinfo(&allsigs, &si)) == -1)
			errmsg_exit1("sigwaitinfo failed, %s\n", ERR_MSG);

		switch (sig) {
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			exit(EXIT_SUCCESS);
		}

		printf("Got signal: %d (%s)\n", sig, strsignal(sig));
		printf("\tsi_signo=%d, si_code=%d (%s), si_value=%d\n",
			si.si_signo, si.si_code,
			(si.si_code == SI_USER) ? "SI_USER" :
			(si.si_code == SI_QUEUE) ? "SI_QUEUE" : "other",
			si.si_value.sival_int);
		printf("\tsi_pid=%d, si_uid=%d\n", si.si_pid, si.si_uid);
	}

	exit(EXIT_SUCCESS);
}
