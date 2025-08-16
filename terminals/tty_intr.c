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
main(int argc, char *argv[])
{
	struct termios to;
	long ic;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [inter-char]\n", argv[0]);

	/* Determine new INTR setting from command line */

	/*
	 * The fpathconf() system calls provide a method for applications to
	 * determine the current value of a configurable system limit or
	 * option variable associated with a pathname or file descriptor.
	 *
	 * The fd argument is an open file descriptor. The name argument
	 * specifies the system variable to be queried. Symbolic constants for
	 * each name value are found in the include file <unistd.h>.
	 *
	 * _PC_VDISABLE:
	 *	Returns the terminal character disabling value.
	 */
	if (argc == 1) {
		if ((ic = fpathconf(STDIN_FILENO, _PC_VDISABLE)) == -1)
			errmsg_exit1("fpathconf failed, %s\n", ERR_MSG);
	} else if (isdigit(argv[1][0])) {
		ic = strtol(argv[1], NULL, 0);	/* Allows hex, octal */
	} else {
		ic = argv[1][0];		/* Literal character */
	}

	/*
	 * Fetch current terminal settings, modify INTR character, and push
	 * changes back to the terminal driver
	 */

	/*
	 * The tcgetattr() function copies the parameters associated with the
	 * terminal referenced by fd in the termios structure referenced by t.
	 * This function is allowed from a background process, however, the
	 * terminal attributes may be subsequently changed by a foreground
	 * process.
	 */
	if (tcgetattr(STDIN_FILENO, &to) == -1)
		errmsg_exit1("tcgetattr failed, %s\n", ERR_MSG);

	to.c_cc[VINTR] = ic;	/* 'VINTR' is define in <sys/_termios.h> */

	/*
	 * The tcsetattr() function sets the parameters associated with the
	 * terminal from the termios structure referenced by t. The action
	 * argument is one of the following values, as specified in the include
	 * file <termios.h>.
	 *
	 * TCSANOW	The change occurs immediately.
	 *
	 * TCSADRAIN	The change occurs after all output written to fd has
	 *		been transmitted to the terminal. This value of
	 *		action should be used when changing parameters that
	 *		affect output.
	 *
	 * TCSAFLUSH	The change occurs after all output written to fd has
	 *		been transmitted to the terminal. Additionally, any
	 *		input that has been received but not read is discarded.
	 */
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &to) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
