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
#include <fcntl.h>
#include <sys/wait.h>

int
main(void)
{
	int fd, flags, status;
	const int foffset = 1000;
	char template[] = "/tmp/temp.XXXXX";
	pid_t pid;

	if ((fd = mkstemp(template)) == -1)
		errmsg_exit1("mkstemp %s, failed, %s\n", template, ERR_MSG);

	printf("File offset before fork(): %ld\n", lseek(fd, 0, SEEK_CUR));

	if ((flags = fcntl(fd, F_GETFL)) == -1)
		errmsg_exit1("fcntl - F_GETFL failed, %s\n", ERR_MSG);
	printf("O_APPEND flag before fork() is: %s\n",
		(flags & O_APPEND) ? "Yes": "No");

	if ((pid = fork()) == -1)
		errmsg_exit1("fork failed, %s\n", ERR_MSG);
	else if (pid == 0) { /* * Child: change file offset and status flags */
		printf("Changes file offset is %d\n", foffset);
		if (lseek(fd, foffset, SEEK_SET) == -1)
			errmsg_exit1("lseek - SEEK_SET failed, %s\n", ERR_MSG);

		printf("changes file flags including 'O_APPEND'.\n");
		if ((flags = fcntl(fd, F_GETFL)) == -1)
			errmsg_exit1("fcntl - F_GETFL failed, %s\n", ERR_MSG);
		flags |= O_APPEND;
		if (fcntl(fd, F_SETFL, flags) == -1)
			errmsg_exit1("fcntl - F_SETFL failed, %s\n", ERR_MSG);

		_exit(EXIT_SUCCESS);
	} else {	/* Parent: can see file changes made by child */
		/*
		 * The wait() function suspends execution of its calling thread until
		 * status information is available for a child process or a signal is
		 * received. 
		 *
		 * If wait() returns due to a stopped, continued, or terminated child
		 * process, the process ID of the child is returned to the calling
		 * process. Otherwise, a value of -1 is returned and errno is set to
		 * indicate the error.
		 */
		if (wait(&status) == -1)	/* Wait for child exit */
			errmsg_exit1("wait failed, %s\n", ERR_MSG);
		printf("Child process has exited, code = %d.\n", WIFEXITED(status));

		printf("File offset in parent process: %ld\n", lseek(fd, 0, SEEK_CUR));

		if ((flags = fcntl(fd, F_GETFL)) == -1)
			errmsg_exit1("fcntl - F_GETFL failed, %s\n", ERR_MSG);
		printf("O_APPEND flag in parent process is: %s\n",
			(flags & O_APPEND) ? "Yes": "No");
	}

	exit(EXIT_SUCCESS);
}
