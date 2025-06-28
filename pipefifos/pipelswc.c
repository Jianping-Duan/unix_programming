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

int
main(void)
{
	int pfd[2];

	if (pipe(pfd) == -1)
		errmsg_exit1("pipe failed, %s\n", ERR_MSG);

	switch (fork()) {
	case -1:
		errmsg_exit1("fork (1) failed, %s\n", ERR_MSG);
	case 0:
		if (close(pfd[0]) == -1)
			errmsg_exit1("close (1) failed, %s\n", ERR_MSG);

		if (pfd[1] != STDOUT_FILENO) {
			if (dup2(pfd[1], STDOUT_FILENO) == -1)
				errmsg_exit1("dup2 (1) failed\n");
			if (close(pfd[1]) == -1)
				errmsg_exit1("close (2) failed\n");
		}

		execlp("/bin/ls", "ls", (char *)NULL);
		errmsg_exit1("execlp - ls failed, %s\n", ERR_MSG);
	default:
		break;
	}

	switch (fork()) {
	case -1:
		errmsg_exit1("fork (2) failed, %s\n", ERR_MSG);
	case 0:
		if (close(pfd[1]) == -1)
			errmsg_exit1("close (3) failed, %s\n", ERR_MSG);

		if (pfd[0] != STDIN_FILENO) {
			if (dup2(pfd[0], STDIN_FILENO) == -1)
				errmsg_exit1("dup2 (2) failed\n");
			if (close(pfd[0]) == -1)
				errmsg_exit1("close (4) failed\n");
		}

		execlp("/usr/bin/wc", "wc", "-l", (char *)NULL);
		errmsg_exit1("execlp - wc failed, %s\n", ERR_MSG);
	default:
		break;
	}

	if (close(pfd[0]) == -1)
		errmsg_exit1("close (5) failed, %s\n", ERR_MSG);
	if (close(pfd[1]) == -1)
		errmsg_exit1("close (6) failed, %s\n", ERR_MSG);

	if (wait(NULL) == -1)
		errmsg_exit1("wait (1) failed\n");
	if (wait(NULL) == -1)
		errmsg_exit1("wait (2) failed\n");

	exit(EXIT_FAILURE);
}
