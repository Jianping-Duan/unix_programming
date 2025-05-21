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

static void sig_handler(int);
static void print_sigset(const sigset_t *);

#define SECS	6

int
main(void)
{
	struct sigaction sact;
	sigset_t blocked, pending;

	printf("Setting up handler for SIGINT\n");

	sigemptyset(&sact.sa_mask);
	sact.sa_handler = sig_handler;
	sact.sa_flags = 0;	/* no need to set */
	/* more details see sigaction(3) */
	if (sigaction(SIGINT, &sact, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	/* 
	 * Block SIGINT for a while.
	 */
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGINT);
	if (sigprocmask(SIG_SETMASK, &blocked, NULL) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);

	printf("Blocking SIGINT %d seconds "
		"(to facilitate enter Control-C)\n", SECS);
	sleep(SECS);

	/*
	 * Display mask of pending signals.
	 */
	if (sigpending(&pending) == -1)
		errmsg_exit1("sigpending failed, %s\n", ERR_MSG);
	printf("Pending signals are: ");
	print_sigset(&pending);

	/* 
	 * Now ignore SIGINT
	 */
	printf("Sleeps %d second(to facilitate enter Control-C)\n", SECS);
	sleep(SECS);
	printf("Ignoring SIGINT.\n");
	if (signal(SIGINT, SIG_IGN) == SIG_ERR)
		errmsg_exit1("signal failed, %s\n", ERR_MSG);

	/* 
	 * Redisplay mask of pending signals.
	 */
	if (sigpending(&pending) == -1)
		errmsg_exit1("sigpending failed, %s\n", ERR_MSG);
	if (sigismember(&pending, SIGINT))
		printf("SIGINT is now pending.\n");
	else {
		printf("Pending signals are: ");
		print_sigset(&pending);
	}

	/* 
	 * Reestablish SIGINT handler.
	 */
	sigemptyset(&sact.sa_mask);
	sact.sa_handler = sig_handler;
	sact.sa_flags = 0;
	if (sigaction(SIGINT, &sact, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);
	
	printf("Sleeps %d second(to facilitate enter Control-C)\n", SECS);
	sleep(SECS);
	
	/*
	 * And unblock SIGINT.
	 */
	printf("Unblock SIGINT.\n");
	sigemptyset(&blocked);
	if (sigprocmask(SIG_SETMASK, &blocked, NULL) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);


	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	printf("Caught signal %d (%s)\n", sig, strsignal(sig));
	/* 
	 * If the stream argument is NULL, 
	 * fflush() flushes all open output streams.
	 * More details see fflush(3)
	 */
	fflush(NULL);
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
