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

static void show_wait_status(int);

int
main(int argc, char *argv[])
{
	int status;
	pid_t cpid;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [exit-status]\n", argv[0]);

	if ((cpid = fork()) == -1)
		errmsg_exit1("fork failed, %s\n", ERR_MSG);
	else if (cpid == 0) {
		/* 
		 * Child: either exits immediately with given
		 * status or loops waiting for signals.
		 */

		printf("Child started with PID = %d\n", getpid());
		if (argc > 1)	/* Status supplied on command-line */
			exit(getint(argv[1]));
		else	/* wait for a signal */
			while (1)
				pause();

		exit(EXIT_FAILURE);	/* no reached */
	} else {
		/* 
		 * Parent: repeatedly wait on child until it
		 * either exits or is terminated by a signal.
		 */

		while (1) {
			/*
			 * For the waitpid() and wait4() functions, the single wpid argument
			 * specifies the set of child processes for which to wait.
			 *
			 * If wpid is -1, the call waits for any child process.
			 * 
			 * If wpid is 0, the call waits for any child process in the process
			 * group of the caller.
			 *
			 * If wpid is greater than zero, the call waits for the process
			 * with process ID wpid.
			 *
			 * If wpid is less than -1, the call waits for any process whose
			 * process group ID equals the absolute value of wpid.
			 *
			 * WCONTINUED:
			 *	Report the status of selected processes that have continued
			 *	from a job control stop by receiving a SIGCONT signal.
			 *
			 * WUNTRACED:
			 *	Report the status of selected processes which are stopped due
			 *	to a SIGTTIN, SIGTTOU, SIGTSTP, or SIGSTOP signal.
			 *
			 * More details see waitpid(2)
			 */
			if ((cpid = waitpid(-1, &status, WUNTRACED | WCONTINUED)) == -1)
				errmsg_exit1("waitpid (-1) failed, %s\n", ERR_MSG);

			/* Print status in hex, and as separate decimal bytes */
			printf("waitpid() returned: PID = %d, status = 0x%04x, (%d, %d)\n",
				cpid, status, status >> 8, status & 0xff);
			show_wait_status(status);

			/*
			 * WIFEXITED(status):
			 *	True if the process terminated normally by a call to _exit(2) or
			 *	exit(3).
			 *
			 * WIFSIGNALED(status):
			 *	True if the process terminated due to receipt of a signal.
			 * 
			 */
			if (WIFEXITED(status) || WIFSIGNALED(status))
				exit(EXIT_SUCCESS);
		}
	}
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
	 *	True if the process has not terminated, and has continued after a
	 *	job control stop.  This macro can be true only if the wait call
	 *	specified the WCONTINUED option.
	 */
	if (WIFEXITED(status))
		printf("child process exited, status = %d\n", WEXITSTATUS(status));
	else if(WIFSIGNALED(status)) {
		printf("child process killed by signal %d (%s)\n", WTERMSIG(status),
			strsignal(WTERMSIG(status)));
		if (WCOREDUMP(status))
			printf(" (core dumped)\n");
	} else if (WIFSTOPPED(status))
		printf("child process stopped by signal %d (%s)\n", WSTOPSIG(status),
			strsignal(WSTOPSIG(status)));
	else if (WIFCONTINUED(status))
		printf("child process continued\n");
	else
		printf("what happend to this child ? (status = 0x%x)\n", status);
}
