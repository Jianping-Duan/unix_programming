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

#define POPEN_FMT	"/bin/ls -d %s" 
#define PAT_SIZE	64
#define PCMD_LEN	(sizeof(POPEN_FMT) + PAT_SIZE)

static void show_wait_status(int);

int
main(void)
{
	char pat[PAT_SIZE], cmd[PCMD_LEN], pathname[_POSIX_PATH_MAX];
	FILE *fp;
	int len, i, cnt, status;
	bool badpat;

	while (1) {
		printf("pattern: ");
		fflush(stdout);
		if (fgets(pat, PAT_SIZE, stdin) == NULL)
			break;
		if ((len = (int)strlen(pat)) <= 1)
			continue;

		if (pat[len - 1] == '\n')	/* Strip trailing newline */
			pat[len - 1] = '\0';

		/*
		 * Ensure that the pattern contains only valid characters,
		 * i.e., letters, digits, underscore, dot, and the shell
		 * globbing characters. (Our definition of valid is more
		 * estrictive than the shell, which permits other characters
		 * to be included in a filename if they are quoted.)
		 */
		for (i = 0, badpat = false; i < len && !badpat; i++)
			if (!isalnum(pat[i]) &&
				strchr("_*?[^-].", pat[i]) == NULL) {
				badpat = true;
			}

		if (badpat) {
			printf("Bad pattern character: %c\n", pat[i - 1]);
			continue;
		}

		/* Build and execute command to glob 'pat' */
		snprintf(cmd, PCMD_LEN, POPEN_FMT, pat);

		/*
		 * The popen() function “opens” a process by creating a
		 * bidirectional pipe forking, and invoking the shell. Any
		 * streams opened by previous popen() calls in the parent
		 * process are closed in the new child process. Historically,
		 * popen() was implemented with a unidirectional pipe; hence
		 * many implementations of popen() only allow the type argument
		 * to specify reading or writing, not both. Since popen() is now
		 * implemented using a bidirectional pipe, the type argument may
		 * request a bidirectional data flow. The type argument is a
		 * pointer to a null-terminated string which must be ‘r’ for
		 * reading, ‘w’ for writing, or ‘r+’ for reading and writing.
		 *
		 * A letter ‘e’ may be appended to that to request that the
		 * underlying file descriptor be set close-on-exec.
		 *
		 * The command argument is a pointer to a null-terminated string
		 * containing a shell command line. This command is passed to
		 * /bin/sh using the -c flag; interpretation, if any, is
		 * performed by the shell.
		 */
		if ((fp = popen(cmd, "r")) == NULL) {
			fprintf(stderr, "popen failed\n");
			continue;
		}

		cnt = 0;
		while (fgets(pathname, _POSIX_PATH_MAX, fp) != NULL) {
			printf("%s", pathname);
			cnt++;
		}

		 /*
		  * Close pipe, fetch and display termination status 
		  *
		  * The pclose() function waits for the associated process to
		  * terminate and returns the exit status of the command as
		  * returned by wait4(2).
		  */
		status = pclose(fp);
		printf("\t%d matching file%s\n", cnt, (cnt != 1) ? "s" : "");
		printf("\tpclose status = %#x\n", (unsigned)status);
		if (status != -1) {
			printf("\t\t");
			show_wait_status(status);
		}
	}

	exit(EXIT_SUCCESS);
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
