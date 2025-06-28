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

#define BUFSIZE		128

int
main(int argc, char *argv[])
{
	int pfd[2];
	char buf[BUFSIZE];
	ssize_t nrd, len;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s string\n", argv[0]);

	/*
	 * The pipe() function creates a pipe, which is an object allowing
	 * bidirectional data flow, and allocates a pair of file descriptors
	 *
	 * By convention, the first descriptor is normally used as the read end
	 * of the pipe, and the second is normally the write end, so that data
	 * written to fildes[1] appears on (i.e., can be read from) fildes[0].
	 * This allows the output of one program to be sent to another program:
	 * the source's standard output is set up to be the write end of the
	 * pipe, and the sink's standard input is set up to be the read end of
	 * the pipe. The pipe itself persists until all its associated
	 * descriptors are closed.
	 */
	if (pipe(pfd) == -1)
		errmsg_exit1("pipe failed, %s\n", ERR_MSG);

	switch (fork()) {
	case -1:
		errmsg_exit1("fork failed, %s\n", ERR_MSG);
	case 0:
		if (close(pfd[1]) == -1)	/* Write end is unused */
			errmsg_exit1("close pipe[1] failed, %s\n", ERR_MSG);
		
		while (1) {
			if ((nrd = read(pfd[0], buf, BUFSIZE)) == -1)
				errmsg_exit1("read pipe[0] failed, %s\n",
					ERR_MSG);
			if (nrd == 0)
				break;
			if (write(STDOUT_FILENO, buf, nrd) != nrd)
				errmsg_exit1("child - partial/failed write\n");
		}

		write(STDOUT_FILENO, "\n", 1);
		if (close(pfd[0]) == -1)
			errmsg_exit1("clsoe pipe[0] failed, %s\n", ERR_MSG);

		exit(EXIT_SUCCESS);
	default:
		if (close(pfd[0]) == -1)	/* Read end is unused */
			errmsg_exit1("close pipe[0] failed, %s\n", ERR_MSG);

		len = (int)strlen(argv[1]);
		if (write(pfd[1], argv[1], len) != len)
			errmsg_exit1("parent - partial/failed write\n");

		if (close(pfd[1]) == -1)
			errmsg_exit1("close pipe[1] failed, %s\n", ERR_MSG);
		wait(NULL);	/* Wait for child to finish */
		exit(EXIT_SUCCESS);
	}
}
