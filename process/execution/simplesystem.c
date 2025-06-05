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

#define CMD_LEN		256

static void show_wait_status(const char *, int);
static int mysystem(const char *);

int
main(void)
{
	 char cmd[CMD_LEN];	/* Command to be executed by system() */
	 int status;	/* Status return from system() */

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
				printf(">>>> (Probably) could not invoke shell\n");
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

static int
mysystem(const char *cmd)
{
	int status;
	pid_t cpid;

	switch (cpid = fork()) {
		case -1:
			return -1;
		case 0:
			execlp("/bin/tcsh", "tcsh", "-c", cmd, (char *)NULL);
			_exit(127);	/* failed execlp */
		default:
			if (waitpid(cpid, &status, 0) == -1)
				return -1;
			return status;
	}
}
