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
#include <signal.h>
#include <termios.h>
#include <fcntl.h>

static volatile sig_atomic_t got_sigio = 0;

static void sig_handler(int);
static int tty_setcbreak(int, struct termios *);

int
main(void)
{
	struct sigaction sa;
	int flags, cnt;
	struct termios oritos;
	bool done;
	char ch;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGIO, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGIO failed, %s\n", ERR_MSG);

	/* Set owner process that is to receive "I/O possible" signal */

	if (fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1)
		errmsg_exit1("fcntl - F_SETOWN failed, %s\n", ERR_MSG);

	/*
	 * Enable "I/O possible" signaling and make I/O nonblocking for file
	 * descriptor.
	 */

	flags = fcntl(STDIN_FILENO, F_GETFL);
	flags |= O_ASYNC | O_NONBLOCK;
	if (fcntl(STDIN_FILENO, F_SETFL, flags) == -1)
		errmsg_exit1("fcntl - F_SETFL failed, %s\n", ERR_MSG);

	/* Place terminal in cbreak mode */

	if (tty_setcbreak(STDIN_FILENO, &oritos) == -1)
		errmsg_exit1("tty_setcbreak error\n");

	for (done = false, cnt = 0; !done; cnt++) {
		usleep(100000);

		if (!got_sigio) {
			cnt++;
			continue;
		}
		got_sigio = 0;

		while (read(STDIN_FILENO, &ch, 1) > 0 && !done) {
			printf("cnt=%d; read %c\n", cnt, ch);
			done = (ch == '#');
		}
	}

	/* Restore original terminal settings */
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &oritos) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	assert(sig == SIGIO);
	got_sigio = 1;
}

int
tty_setcbreak(int fd, struct termios *prevto)
{
	struct termios tos;

	if (tcgetattr(fd, &tos) == -1)
		return -1;

	if (prevto != NULL)
		*prevto = tos;

	tos.c_lflag &= ~(ICANON | ECHO);
	tos.c_lflag |= ISIG;

	tos.c_iflag &= ~ICRNL;

	tos.c_cc[VMIN] = 1;	/* Character-at-a-time input */
	tos.c_cc[VTIME] = 0;	/* with blocking */

	if (tcsetattr(fd, TCSAFLUSH, &tos) == -1)
		return -1;

	return 0;
}
