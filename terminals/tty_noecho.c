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
#include <termios.h>

int
main(void)
{
	struct termios to, save;
	char buf[BUF_SIZE];

	/* Retrieve current terminal settings, turn echoing off */

	if (tcgetattr(STDIN_FILENO, &to) == -1)
		errmsg_exit1("tcgetattr failed, %s\n", ERR_MSG);
	save = to;

	to.c_lflag &= ~ECHO;		/* ECHO off, other bits unchanged */
	if  (tcsetattr(STDIN_FILENO, TCSAFLUSH, &to) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);

	/* Read some input and then display it back to the user */

	printf("Enter text: ");
	fflush(stdout);
	if (fgets(buf, BUF_SIZE, stdin) == NULL)
		fprintf(stderr, "Got end-of-file/error on fgets()\n");
	else
		printf("\nRead: %s\n", buf);

	/* Restore original terminal settings */

	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
