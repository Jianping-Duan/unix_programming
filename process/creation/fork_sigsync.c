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
#include <time.h>	/* time(), localtime(), strftime() */

#define SIG_SYNC	SIGUSR1		/* Synchronization signal */

static void sig_handler(int);
static char * currtime(const char *);

int
main(void)
{
	pid_t cpid;
	sigset_t bmask, omask, emask;
	struct sigaction sa;

	sigemptyset(&bmask);
	sigaddset(&bmask, SIG_SYNC);
	if (sigprocmask(SIG_SETMASK, &bmask, &omask) == -1)
		errmsg_exit1("sigprocmask - SIG_SETMASK failed, %s\n", ERR_MSG);

	/*
	 * If a signal is caught during the system calls listed below, the call may
	 * be forced to terminate with the error EINTR, the call may return with a
	 * data transfer shorter than requested, or the call may be restarted.
	 * Restart of pending calls is requested by setting the SA_RESTART bit in
	 * sa_flags. The affected system calls include open(2), read(2), write(2),
	 * sendto(2), recvfrom(2), sendmsg(2) and recvmsg(2) on a communications
	 * channel or a slow device (such as a terminal, but not a regular file) and
	 * during a wait(2) or ioctl(2). However, calls that have already committed
	 * are not restarted, but instead return a partial success (for example, a
	 * short read count).
	 * 
	 * After a fork(2) or vfork(2) all signals, the signal mask, the signal
	 * stack, and the restart/interrupt flags are inherited by the child.
	 */
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIG_SYNC, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGUSR1 failed, %s\n", ERR_MSG);

	if ((cpid = fork()) == -1)
		errmsg_exit1("fork failed, %s\n", ERR_MSG);
	else if (cpid == 0) {	/* Child */
		 /* Child does some required action here... */
		 printf("[%s %d] Child started - doing some work.\n",
			currtime("%T"), getpid());
		 sleep(2);	/* Simulate time spent doing some work */

		 /* And then signals parent that it's done */
		 printf("[%s %d] Child about to signal parent.\n",
			currtime("%T"), getpid());
		 if (kill(getppid(), SIG_SYNC) == -1)
			 errmsg_exit1("kill - SIGUSR1 failed, %s\n", ERR_MSG);

		/* Now child can do other things... */
		printf("\t[%s %d] Child can do other things...\n",
			currtime("%T"), getpid());

		_exit(EXIT_SUCCESS);
	} else {	/* Parent */
		/* 
		 * Parent may do some work here, and then waits for child to
		 * complete the required action.
		 */
		printf("[%s %d] Parent about to wait for signal.\n",
			currtime("%T"), getpid());
		sigemptyset(&emask);
		if (sigsuspend(&emask) == -1 && errno != EINTR)
			errmsg_exit1("sigsuspend failed, %s\n", ERR_MSG);
		printf("[%s %d] Parent got signal.\n", currtime("%T"), getpid());

		/* If required, return signal mask to its original state */
		if (sigprocmask(SIG_SETMASK, &omask, NULL) == -1)
			errmsg_exit1("sigprocmask - SIG_SETMAKS failed, %s\n", ERR_MSG);

		/* Parent carries on to do other things... */
		printf("\t[%s %d] Parent carries on to do other things...\n",
			currtime("%T"), getpid());

		exit(EXIT_SUCCESS);
	}
}

static void
sig_handler(int sig)
{
	printf("[%s %d] Caught signal: %d (%s)\n", currtime("%T"), getpid(), sig,
		strsignal(sig));
}

static char *
currtime(const char *fmt)
{
#define BUFSIZE	512
	static char buf[BUFSIZE];
	time_t tim;
	size_t sz;
	struct tm *tmptr;

	tim = time(NULL);
	if ((tmptr = localtime(&tim)) == NULL) {
		fprintf(stderr, "localtime failed, %s\n", ERR_MSG);
		return NULL;
	}

	/* more formats see strftime(3) */
	sz = strftime(buf, BUFSIZE, (fmt != NULL) ? fmt : "%c", tmptr);	
	return ((sz == 0) ? NULL : buf);

}
