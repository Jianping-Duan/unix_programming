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

static volatile int handler_sleep;
static volatile int sigcnt = 0;		/* Number of signals received */
static volatile sig_atomic_t alldone = 0;

static void siginfo_handler(int, siginfo_t *, void *);

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	int sig;
	sigset_t bmask, omask;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [block-time [handler-sleep-time]]\n",
			argv[0]);

	printf("%s: PID is %d\n", argv[0], getpid());

	handler_sleep = (argc > 2) ? getint(argv[2]) : 1;

	/*
	 * Establish handler for most signals. During execution of the handler,
	 * mask all other signals to prevent handlers recursively interrupting
	 * each other (which would make the output hard to read).
	 */

	sigfillset(&sa.sa_mask);
	/*
	 * SA_SIGINFO:
	 *	If this bit is set, the handler function is assumed to be
	 *	pointed to by the sa_sigaction member of struct sigaction and
	 *	should match the prototype shown above or as below in EXAMPLES.
	 *	This bit should not be set when assigning SIG_DFL or SIG_IGN.
	 */
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = siginfo_handler;
	for (sig = 1; sig < NSIG; sig++)
		if (sig != SIGTSTP && sig != SIGQUIT)
			sigaction(sig, &sa, NULL);
	
	/* 
	 * Optionally block signals and sleep, allowing signals to be
	 * sent to us before they are unblocked and handled.
	 */

	if (argc <= 1)
		goto nosleep;
	
	sigfillset(&bmask);
	sigdelset(&bmask, SIGINT);	/* interrupt program */
	sigdelset(&bmask, SIGTERM);	/* software termination signal */
	if (sigprocmask(SIG_SETMASK, &bmask, &omask) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);

	printf("%s: signals blocked - sleep %s seconds\n", argv[0], argv[1]);
	sleep(getint(argv[1]));
	printf("%s: sleep complete.\n", argv[0]);

	if (sigprocmask(SIG_SETMASK, &omask, NULL) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);

nosleep:
	while (!alldone)
		pause();
	printf("Caught %d signals.\n", sigcnt);

	exit(EXIT_SUCCESS);
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GCC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
static void
siginfo_handler(int sig, siginfo_t *si, void *ucontext)
{
	/* SIGINT or SIGTERM can be used to terminate program */
	if (sig == SIGINT || sig == SIGTERM) {
		alldone = 1;
		return;
	}

	sigcnt++;
	printf("caugth signal %d (%s)\n", sig, strsignal(sig));

	/* more details see '/usr/include/sys/signal.h' */
	printf("\tsi_signo=%d, si_code=%d (%s), ", si->si_signo, si->si_code,
		(si->si_code == SI_USER) ? "SI_USER" :
		(si->si_code == SI_QUEUE) ? "SI_QUEUE" : "other");
	printf("si_value=%d\n", si->si_value.sival_int);
	printf("\tsi_pid=%d, si_uid=%d\n", si->si_pid, si->si_uid);

	sleep(handler_sleep);
}
