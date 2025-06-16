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

static int sigcnt[NSIG];	/* Counts deliveries of each signal */
/* Set nonzero if SIGINT is delivered */
static volatile sig_atomic_t gotsigint = 0;

static void sig_handler(int);
static void print_sigset(const sigset_t *);

int
main(int argc, char *argv[])
{
	int i, secs;
	sigset_t bmask, pmask, emask;

	printf("%s: PID is %d\n", argv[0], getpid());

	/* More signals see /usr/include/sys/signal.h */
	for (i = 1; i < NSIG; i++)
		(void)signal(i, sig_handler);	/* Ignore errors */

	/* 
	 * If a sleep time was specified, temporarily block all signals,
	 * sleep (while another process sends us signals), and then
	 * display the mask of pending signals and unblock all signals.
	 */
	if (argc == 1)
		goto nosleep;

	secs = (int)getlong(argv[1], GN_GT_0);

	/* initializes a signal set to contain all signals. */
	sigfillset(&bmask);
	/* 
	 * changes the current signal mask
	 * (those signals that are blocked from delivery).
	 * Signals are blocked if they are members of the current signal mask
	 * set.
	 */
	if (sigprocmask(SIG_SETMASK, &bmask, NULL) == -1)
		errmsg_exit1("sigprocmask(SIG_SETMASK,..) failed, %s\n",
			ERR_MSG);

	printf("%s sleeping for %d seconds.\n", argv[0], secs);
	sleep(secs);

	/* 
	 * The sigpending() system call returns a mask of the signals pending
	 * for delivery to the calling thread or the calling process in the
	 * location indicated by set. Signals may be pending because they are
	 * currently masked, or transiently before delivery (although the latter
	 * case is not normally detectable).
	 */
	if (sigpending(&pmask) == -1)
		errmsg_exit1("sigpending failed, %s\n", ERR_MSG);
	printf("%s pending signals are:\n", argv[0]);
	print_sigset(&pmask);

	/* 
	 * Initializes a signal set to be empty.
	 * Unblock all signals.
	 */
	sigemptyset(&emask);
	if (sigprocmask(SIG_SETMASK, &emask, NULL) == -1)
		errmsg_exit1("sigprocmask(SIG_SETMASK,..) failed, %s\n",
			ERR_MSG);

nosleep:
	while (!gotsigint)	/* Loop until SIGINT caught */
		continue;

	for (i = 1; i < NSIG; i++)
		if (sigcnt[i] != 0)
			printf("%s: signal %d caught %d times\n", argv[0], i,
				sigcnt[i]);

	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	if (sig == SIGINT)
		gotsigint = 1;
	else
		sigcnt[sig]++;
}

static void
print_sigset(const sigset_t *sigs)
{
	int sig, cnt = 0;

	for (sig = 1; sig < NSIG; sig++) {
		/* 
		 * returns whether a specified signal signo is contained in
		 * the signal set.
		 */
		if (sigismember(sigs, sig)) {
			cnt++;
			printf("\t\t%d (%s)\n", sig, strsignal(sig));
		}
	}

	if (cnt == 0)
		printf("\t\t<empty signal set>\n");
}
