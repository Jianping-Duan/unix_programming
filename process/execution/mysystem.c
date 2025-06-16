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

#define CMD_LEN		256

static void show_wait_status(const char *, int);
static int mysystem(const char *);

int
main(void)
{
	 char cmd[CMD_LEN];	/* Command to be executed by system() */
	 int status;		/* Status return from system() */

	 while (1) {
		printf("Command (myquit to quit)> ");
		fflush(stdout);

		if (fgets(cmd, CMD_LEN, stdin) == NULL)
			 break;

		if (cmd[0] == '\n' || cmd[0] == '\0')
			continue;
		if (strcmp(cmd, "myquit\n") == 0)
			break;

		status = mysystem(cmd);
		printf(">>>> system() returned: status=0x%04x (%d, %d)\n",
			(unsigned)status, status >> 8, status & 0xff);
		if (status == -1)
			errmsg_exit1(">>>> system failed, %s\n", ERR_MSG);
		else {
			if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
				printf(">>>> (Probably) could not invoke "
					"shell\n");
			else
				show_wait_status(">>>> ", status);
		}
	 }

	exit(EXIT_SUCCESS);
}

static void
show_wait_status(const char *msg, int status)
{
	if (msg != NULL)
		printf("%s", msg);

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

static int
mysystem(const char *cmd)
{
	sigset_t bmask, omask;
	struct sigaction saign, saoint, saoquit, sadfl;
	int status, save_errno;
	pid_t cpid;

	if (cmd == NULL)
		return system(":") == 0;	/* Is a shell available ? */

	/*
	 * The parent process (the caller of system()) blocks SIGCHLD
	 * and ignore SIGINT and SIGQUIT while the child is executing.
	 * We must change the signal settings prior to forking, to avoid
	 * possible race conditions. This means that we must undo the
	 * effects of the following in the child after fork(). 
	 */

	sigemptyset(&bmask);
	sigaddset(&bmask, SIGCHLD);	/* Block SIGCHLD */
	if (sigprocmask(SIG_BLOCK, &bmask, &omask) == -1)
		errmsg_exit1("sigprocmask - SIG_BLOCK failed, %s\n", ERR_MSG);
	
	sigemptyset(&saign.sa_mask);
	saign.sa_flags = 0;
	saign.sa_handler = SIG_IGN;
	if (sigaction(SIGINT, &saign, &saoint) == -1)
		errmsg_exit1("sigaction - SIGINT failed, %s\n", ERR_MSG);
	if (sigaction(SIGQUIT, &saign, &saoquit) == -1)
		errmsg_exit1("sigaction - SIGQUIT failed, %s\n", ERR_MSG);

	switch (cpid = fork()) {
	case -1:
		status = -1;
		break;
	case 0:
		/*
		 * We ignore possible error returns because the only specified
		 * error is for a failed exec(), and because errors in these
		 * calls can't affect the caller of system() (which is a
		 * separate process)
		 */

		sigemptyset(&sadfl.sa_mask);
		sadfl.sa_handler = SIG_DFL;
		sadfl.sa_flags = 0;
		if (saoint.sa_handler != SIG_IGN)
			if (sigaction(SIGINT, &sadfl, NULL) == -1)
				errmsg_exit1("sigaction - SIGINT failed, %s\n",
					ERR_MSG);
		if (saoquit.sa_handler != SIG_IGN)
			if (sigaction(SIGQUIT, &sadfl, NULL) == -1)
				errmsg_exit1("sigaction - SIGQUIT failed, %s\n",
					ERR_MSG);

		if (sigprocmask(SIG_SETMASK, &omask, NULL) == -1)
			errmsg_exit1("sigprocmask - SIG_SETMASK failed, %s\n",
				ERR_MSG);

		execlp("/bin/tcsh", "tcsh", "-c", cmd, (char *)NULL);
		_exit(127);
	default:	/* Parent: wait for our child to terminate */
		/*
		 * We must use waitpid() for this task; using wait() could
		 * inadvertently collect the status of one of the caller's other
		 * children.
		 *
		 * If waitpid() returns due to a stopped, continued, or
		 * terminated child process, the process ID of the child is
		 * returned to the calling process. If there are no children not
		 * previously awaited, -1 is returned with errno set to ECHILD.
		 * Otherwise, if WNOHANG is specified and there are no stopped,
		 * continued or exited children, 0 is returned. If an error is
		 * detected or a caught signal aborts the call, a value of -1 is
		 * returned and errno is set to indicate the error.
		 */
		while(waitpid(cpid, &status, 0) == -1)
			if (errno != EINTR) {
				status = -1;
				break;
			}
		break;
	}

	/* Unblock SIGCHLD, restore dispositions of SIGINT and SIGQUIT */
	save_errno = errno;
	if (sigprocmask(SIG_SETMASK, &omask, NULL) == -1)
		errmsg_exit1("sigprocmask - SIG_SETMASK failed, %s\n", ERR_MSG);
	if (sigaction(SIGINT, &saoint, NULL) == -1)
		errmsg_exit1("sigaction - SIGINT failed, %s\n", ERR_MSG);
	if (sigaction(SIGQUIT, &saoquit, NULL) == -1)
		errmsg_exit1("sigaction - SIGQUIT failed, %s\n", ERR_MSG);
	errno = save_errno;

	return status;
}
