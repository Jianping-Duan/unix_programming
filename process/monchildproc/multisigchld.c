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
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

/* Number of children started but not yet waited on */
static volatile int children = 0;

static char * currtime(const char *);
static void show_wait_status(int);
static void sigchld_handler(int);

int
main(int argc, char *argv[])
{
	int i, sigcnt;
	struct sigaction sa;
	sigset_t bmask, emask;

	if (argc < 2)
		errmsg_exit1("Usage: %s child-sleep-time...\n", argv[0]);

	sigcnt = 0;	
	children = argc - 1;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sigchld_handler;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGCHLD failed, %s\n", ERR_MSG);

	/* 
	 * lock SIGCHLD to prevent its delivery if a child terminates
	 * before the parent commences the sigsuspend() loop below.
	 */
	sigemptyset(&bmask);
	sigaddset(&bmask, SIGCHLD);
	if (sigprocmask(SIG_SETMASK, &bmask, NULL) == -1)
		errmsg_exit1("sigprocmask - SIG_SETMASK failed, %s\n", ERR_MSG);

	/* Create one child process for each command-line argument */
	for (i = 1; i < argc; i++)
		switch (fork()) {
		case -1:
			errmsg_exit1("fork failed, %s\n", ERR_MSG);
		case 0:
			sleep(getint(argv[i]));
			printf("[%s] child %d (PID = %d) exiting\n",
				currtime("%T"), i, getpid());
			_exit(EXIT_SUCCESS);
		default:	/* Parent - loops to create next child */
			break;
		}

	/* Parent comes here: wait for SIGCHLD until all children are dead */
	sigemptyset(&emask);
	while (children > 0) {
		if (sigsuspend(&emask) == -1 && errno != EINTR)
			errmsg_exit1("sigsuspend failed, %s\n", ERR_MSG);
		sigcnt++;
	}

	printf("[%s] All %d children processes have terminated; SIGCHLD was "
		"caught %d times\n", currtime("%T"), children, sigcnt);

	exit(EXIT_SUCCESS);
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

static void
show_wait_status(int status)
{
	/*
	 * WCOREDUMP(status):
	 *	If WIFSIGNALED(status) is true, evaluates as true if the
	 *	termination of the process was accompanied by the creation of a
	 *	core file containing an image of the process when the signal was
	 *	received.
	 *
	 * WIFSTOPPED(status):
	 *	True if the process has not terminated, but has stopped and can
	 *	be restarted. This macro can be true only if the wait call
	 *	specified the WUNTRACED option or if the child process is being
	 *	traced (see ptrace(2)).
	 *
	 * WIFCONTINUED(status):
	 *	True if the process has not terminated, and has continued after
	 *	a job control stop. This macro can be true only if the wait call
	 *	specified the WCONTINUED option.
	 */
	if (WIFEXITED(status))
		printf("child process exited, status = %d\n",
			WEXITSTATUS(status));
	else if(WIFSIGNALED(status)) {
		printf("child process killed by signal %d (%s)\n",
			WTERMSIG(status), strsignal(WTERMSIG(status)));
		if (WCOREDUMP(status))
			printf(" (core dumped)\n");
	} else if (WIFSTOPPED(status))
		printf("child process stopped by signal %d (%s)\n",
			WSTOPSIG(status), strsignal(WSTOPSIG(status)));
	else if (WIFCONTINUED(status))
		printf("child process continued\n");
	else
		printf("what happend to this child ? (status = 0x%x)\n",
			status);
}

static void
sigchld_handler(int sig)
{
	int status, save_errno;
	pid_t cpid;

	save_errno = errno;	/* In case we modify 'errno' */

	printf("[%s] Handler: caught SIGCHLD (%d, %s).\n", currtime("%T"),
		sig, strsignal(sig));

	/* Do nonblocking waits until no more dead children are found */
	while ((cpid = waitpid(-1, &status, WNOHANG)) > 0) {
		printf("[%s] Handler: reaped child %d -\n", currtime("%T"),
			cpid);
		printf("\t");
		show_wait_status(status);
		children--;
	}

	if (cpid == -1 && errno != ECHILD)
		errmsg_exit1("waitpid (-1, WNOHANG) failed, %s\n", ERR_MSG);

	sleep(5);	/* Artificially lengthen execution of handler */
	printf("[%s] Handler: returnning\n", currtime("%T"));

	errno = save_errno;
}
