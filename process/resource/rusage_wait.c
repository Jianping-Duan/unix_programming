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
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

static void sig_handler(int);
static void print_child_rusage(const char *);

int
main(int argc, char *argv[])
{
	sigset_t mask;
	struct sigaction sa;
	clock_t start;
	int nsecs;	/* Amount of CPU to consume in child */

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s num-seconds\n", argv[0]);
	if ((nsecs = getint(argv[1])) <= 0)
		errmsg_exit1("The argument (%s) must be greater than 0.\n", argv[1]);

	setvbuf(stdout, NULL, _IONBF, 0);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGUSR1 failed, %s\n", ERR_MSG);

	/*
	 * Child informs parent of impending termination using a signal;
	 * block that signal until the parent is ready to catch it.
	 */
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
		errmsg_exit1("sigprocmask - SIG_BLOCK failed, %s\n", ERR_MSG);

	switch (fork()) {
		case -1:
			errmsg_exit1("fork failed, %s\n", ERR_MSG);
		case 0:
			printf("Child process starting...\n");
			start = clock();
			while (clock() - start < nsecs + CLOCKS_PER_SEC)
				continue;	/* Burn 'nsecs' seconds of CPU time */
			printf("Child process terminating.\n");

			/* Tell parent we're nearly done */
			if (kill(getppid(), SIGUSR1) == -1)
				errmsg_exit1("kill failed, %s\n", ERR_MSG);

			_exit(EXIT_SUCCESS);
		default:
			/* Wait for signal from child */
			sigemptyset(&mask);
			if (sigsuspend(&mask) == -1 && errno != EINTR)
				errmsg_exit1("sigsuspend failed, %s\n", ERR_MSG);
			
			sleep(3);	/* Allow child a bit more time to terminate */

			print_child_rusage("Before wait: ");
			if (wait(NULL) == -1)
				errmsg_exit1("wait failed, %s\n", ERR_MSG);
			print_child_rusage("After wait: ");

			exit(EXIT_SUCCESS);
	}
}

static void
sig_handler(int sig)
{
	printf("Caught signal %d (%s)\n", sig, strsignal(sig));
}

static void
print_child_rusage(const char *msg)
{
	struct rusage ru;

	printf("%s", msg);
	/*
	 * The getrusage() system call returns information describing the resources
	 * utilized by the current thread, the current process, or all its
	 * terminated child processes. The who argument is either RUSAGE_THREAD,
	 * RUSAGE_SELF, or RUSAGE_CHILDREN. 
	 *
	 * More details see getrusage(2).
	 */
	if (getrusage(RUSAGE_CHILDREN, &ru) == -1)
		errmsg_exit1("getrusage - RUSAGE_CHILDREN failed, %s\n", ERR_MSG);
	printf("User CPU: %.2f secs; System CPU; %.2f secs\n",
		ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0,
		ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0);
}
