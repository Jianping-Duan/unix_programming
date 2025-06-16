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
#include <time.h>	/* time() */

static enum {USE_PAUSE = 0, USE_SUSPEND} pause_suspend;
static volatile int sigint_cnt = 0;
static volatile sig_atomic_t got_sigquit = 0;

static void sig_handler(int);
static void print_sigset(const sigset_t *);
static void print_sigmask(const char *);
static void print_pendsigs(const char *);

int
main(int argc, char *argv[])
{
	int flag, sleep_time = 0, loops;
	sigset_t bmask, omask;
	struct sigaction sa;
	time_t stim;

	if (argc < 2) {
		errmsg_exit1("Usage: %s suspend-method [sleep-time]\n"
			"\t0: sigprocmask() + pause(); 1: sigsuspend()\n",
			argv[0]);
	}
	
	flag = getint(argv[1]);
	if (flag != USE_PAUSE && flag != USE_SUSPEND) {
		fprintf(stderr, "Invalid argument, %s\n", argv[1]);
		errmsg_exit1("Usage: %s suspend-method\n"
			"\t0: sigprocmask() + pause(); 1: sigsuspend()\n",
			argv[0]);
	}

	pause_suspend = (flag == 0) ? USE_PAUSE : USE_SUSPEND;
	if (pause_suspend == USE_PAUSE && argc < 3)
		errmsg_exit1("Using sigprocmask() + pause() must be specify "
			"the sleep-time.\n");
	
	if (argc >= 3 && pause_suspend == USE_PAUSE)
		if ((sleep_time = getint(argv[2])) <= 0)
			errmsg_exit1("Argument sleep-time must be greater "
				"than 0\n");

	print_sigmask("Initial signal mask is:\n");

	/* 
	 * Block SIGINT and SIGQUIT - at this point we assume that these signals
	 * are not already blocked (obviously true in this simple program) so
	 * that 'omask' will not contain either of these signals after the call.
	 */
	sigemptyset(&bmask);
	sigaddset(&bmask, SIGINT);
	sigaddset(&bmask, SIGQUIT);
	if (sigprocmask(SIG_BLOCK, &bmask, &omask) == -1)
		errmsg_exit1("sigprocmask - SIG_BLOCK failed, %s\n", ERR_MSG);

	/* Set up handlers for SIGINT and SIGQUIT */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGINT failed, %s\n", ERR_MSG);
	if (sigaction(SIGQUIT, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGQUIT failed, %s\n", ERR_MSG);

	 /* Loop until SIGQUIT received */
	 for (loops = 1; !got_sigquit; loops++) {
		 printf("=== LOOP %d\n", loops);

		 /* Simulate a critical section by delaying a few seconds */
		 print_sigmask("Starting critical section, signal mask is:\n");
		 stim = time(NULL);
		 while (time(NULL) < stim + 4)
			 continue;	/* Run for a few seconds elapsed time */

		if (pause_suspend == USE_SUSPEND) {
			/* 
			 * The right way: use sigsuspend() to atomically unblock
			 * signals and pause waiting for signal.
			 */

			print_pendsigs("Before sigsuspend() - pending signals:\n");
			/*
			 * The sigsuspend() system call temporarily changes the
			 * blocked signal mask to the set to which sigmask
			 * points, and then waits for a signal to arrive; on
			 * return the previous set of masked signals is
			 * restored. The signal mask set is usually empty to
			 * indicate that all signals are to be unblocked for
			 * the duration of the call.
			 *
			 * The sigsuspend() system call always terminates by
			 * being interrupted, returning -1 with errno set to
			 * EINTR.
			 */
			if (sigsuspend(&omask) == -1 && errno != EINTR)
				errmsg_exit1("sigsuspend failed, %s\n", ERR_MSG);
		} else {	/* pause_suspend == USE_SUSPEND */
			/* 
			 * The wrong way: unblock signal using sigprocmask(),
			 * then pause()
			 */

			if (sigprocmask(SIG_SETMASK, &omask, NULL) == -1)
				errmsg_exit1("sigprocmask - SIG_SETMASK "
					"failed, %s\n", ERR_MSG);

			 /* 
			  * At this point, if SIGINT arrives, it will be caught
			  * and handled before the pause() call and, in
			  * consequence, pause() will block. (And thus only
			  * another SIGINT signal AFTER the pause call() will
			  * actually cause the pause() call to be interrupted.)
			  * Here we make the window between the two calls a bit
			  * larger so that we have a better chance of sending
			  * the signal.
			  */

			 printf("Unblocked SIGINT, now wait for %d seconds\n",
				sleep_time);
			 stim = time(NULL);
			 while (time(NULL) < stim + sleep_time)
				 continue;
			printf("Finished waitting - now going to pause()\n");
			/* And now wait for the signal */
			pause();

			printf("Signal count = %d\n", sigint_cnt);
			sigint_cnt = 0;
		}
	 }

	/* Restore signal mask so that signals are unblocked */
	if (sigprocmask(SIG_SETMASK, &omask, NULL) == -1)
		errmsg_exit1("sigprocmask - SIG_SETMASK failed, %s\n", ERR_MSG);
	
	print_sigmask("=== Exited loop\nRestored signal mask to:\n");

	/* Do other processing */

	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	printf("Caught signal %d (%s)\n", sig, strsignal(sig));
	
	if (sig == SIGQUIT) 
		got_sigquit = 1;
	sigint_cnt++;
}

static void
print_sigset(const sigset_t *sigs)
{
	int sig, cnt = 0;

	for (sig = 1; sig < NSIG; sig++)
		if (sigismember(sigs, sig)) {
			cnt++;
			printf("\t\t%d (%s)\n", sig, strsignal(sig));
		}

	if (cnt == 0)
		printf("\t\t<empty signal set>\n");
}

static void 
print_sigmask(const char *msg)
{
	sigset_t currmask;

	if (msg != NULL)
		printf("%s", msg);

	if (sigprocmask(SIG_BLOCK, NULL, &currmask) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);
	print_sigset(&currmask);
}

static void 
print_pendsigs(const char *msg)
{
	sigset_t pending;

	if (msg != NULL)
		printf("%s", msg);
	
	if (sigpending(&pending) == -1)
		errmsg_exit1("sigpending failed, %s\n", ERR_MSG);
	print_sigset(&pending);
}
