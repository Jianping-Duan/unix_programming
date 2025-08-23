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
#include <libgen.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tty_funcs.h"
#include "pty_fork.h"

static struct termios ttyorgin;

static inline void ttyreset(void);

int
main(int argc, char *argv[])
{
	pid_t cpid;
	int mfd, sfd, flags;
	mode_t perms;
	struct winsize ws;
	char slname[MAX_SNAME], buf[BUF_SIZE], *shell;
	fd_set infds;
	ssize_t nrd;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s outfile\n", argv[0]);

	/* Retrieve the attributes of terminal on which we are started */

	if (tcgetattr(STDIN_FILENO, &ttyorgin) == -1)
		errmsg_exit1("tcgetattr failed, %s\n", ERR_MSG);
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1)
		errmsg_exit1("ioctl failed, %s\n", ERR_MSG);

	/*
	 * Create a child process, with parent and child connected via a pty
	 * pair. The child is connected to the pty slave and its terminal
	 * attributes are set to be the same as those retrieved above.
	 */

	cpid = pty_fork(&mfd, slname, MAX_SNAME, &ttyorgin, &ws);
	if (cpid == -1)
		errmsg_exit1("pty_fork failed\n");

	if (cpid == 0) {
		/*
		 * If the SHELL variable is set, use its value to determine the
		 * shell execed in child. Otherwise use /bin/sh.
		 */
		shell = getenv("SHELL");
		if (shell == NULL || *shell == '\0')
			shell = "/bin/sh";
		execlp(shell, shell, (char *)NULL);
		errmsg_exit1("execlp failed, %s\n", ERR_MSG);
	}

	/* Parent: relay data between terminal and pty master */

	flags = O_WRONLY | O_CREAT | O_TRUNC;
	perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	if ((sfd = open(argv[1], flags, perms)) == -1)
		errmsg_exit1("open (%s) failed, %s\n", ERR_MSG);

	/*
	 * Place terminal in raw mode so that we can pass all terminal input to
	 * the pseudoterminal master untouched
	 */

	tty_setraw(STDIN_FILENO, &ttyorgin);

	if (atexit(ttyreset) == -1)
		errmsg_exit1("atexit failed, %s\n", ERR_MSG);

	/*
	 * Loop monitoring terminal and pty master for input. If the terminal
	 * is ready for input, then read some bytes and write them to the pty
	 * master. If the pty master is ready for input, then read some bytes
	 * and write them to the terminal.
	 */

	for (;;) {
		FD_ZERO(&infds);
		FD_SET(STDIN_FILENO, &infds);
		FD_SET(mfd, &infds);

		if (select(mfd + 1, &infds, NULL, NULL, NULL) == -1)
			errmsg_exit1("select failed, %s\n", ERR_MSG);

		if (FD_ISSET(STDIN_FILENO, &infds)) {
			if ((nrd = read(STDIN_FILENO, buf, BUF_SIZE)) == -1)
				errmsg_exit1("read (1) failed, %s\n", ERR_MSG);
			if (write(mfd, buf, nrd) != nrd)
				errmsg_exit1("write (1) failed, %s\n", ERR_MSG);
		}

		if (FD_ISSET(mfd, &infds)) {
			if ((nrd = read(mfd, buf, BUF_SIZE)) == -1)
				errmsg_exit1("read (2) failed, %s\n", ERR_MSG);

			if (write(STDOUT_FILENO, buf, nrd) != nrd)
				errmsg_exit1("write (2) failed, %s\n", ERR_MSG);
			if (write(sfd, buf, nrd) != nrd)
				errmsg_exit1("write (3) failed, %s\n", ERR_MSG);

		}
	}

	exit(EXIT_SUCCESS);
}

static inline void
ttyreset(void)
{
	if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyorgin) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);
}
