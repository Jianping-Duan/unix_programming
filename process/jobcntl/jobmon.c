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
#include <fcntl.h>

static int cmdnum;

static void sig_handler(int);

int
main(void)
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGINT failed, %s\n", ERR_MSG);
	if (sigaction(SIGTSTP, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGTSTP failed, %s\n", ERR_MSG);
	if (sigaction(SIGCONT, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGCONT failed, %s\n", ERR_MSG);

	/*
	 * If stdin is a terminal, this is the first process in pipeline:
	 * print a heading and initialize message to be sent down pipe
	 *
	 * The isatty() function determines if the file descriptor fd refers to a
	 * valid terminal type device.
	 */
	if (isatty(STDIN_FILENO)) {
		fprintf(stderr, "Terminal foreground process group: %d\n",
			tcgetpgrp(STDIN_FILENO));
		fprintf(stderr, "Command\tPID\tPPID\tPGRP\tSID\n");
		cmdnum = 0;
	} else {	/* Not first in pipeline, so read message from pipe */
		if (read(STDIN_FILENO, &cmdnum, sizeof (cmdnum)) <= 0)
			errmsg_exit1("read failed, %s\n", ERR_MSG);
	}

	cmdnum++;
	fprintf(stderr, "%4d\t%5d\t%5d\t%5d\t%5d\n", cmdnum, getpid(), getppid(),
		getpgrp(), getsid(0));

	 /* If not the last process, pass a message to the next process */
	 if (!isatty(STDOUT_FILENO))	/* If not tty, then should be pipe */
		 if (write(STDOUT_FILENO, &cmdnum, sizeof (cmdnum)) == -1)
			 errmsg_exit1("write - STDOUT_FILENO failed, %s\n", ERR_MSG);

	while (1)	/* Wait for signals */
		pause();
}

static void
sig_handler(int sig)
{
	if (getpid() == getpgrp())	/* If process group leader */
		fprintf(stderr, "Terminal foreground process group: %d\n",
			tcgetpgrp(STDERR_FILENO));
	fprintf(stderr, "Process %d (%d) recevied signal %d (%s)\n", getpid(),
		cmdnum, sig, strsignal(sig));

	/*
	 * If we catch SIGTSTP, it won't actually stop us.
	 * Therefore we raise SIGSTOP so we actually get stopped.
	 */
	if (sig == SIGTSTP)
		raise(SIGSTOP);
}
