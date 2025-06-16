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

static void sig_handler(int);

int
main(int argc, char *argv[])
{
	pid_t cpid;
	struct sigaction sa;
	int i;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s {d|s}...[ > sig.log ]\n", argv[0]);

	setbuf(stdout, NULL);	 /* Make stdout unbuffered */

	/*
	 * The tcgetpgrp() function returns the value of the process group ID of
	 * the foreground process group associated with the terminal device. If
	 * there is no foreground process group, tcgetpgrp() returns an invalid
	 * process ID.
	 */
	printf("PID of parent process is %d\n", getpid());
	printf("Foreground process group ID is %d\n", tcgetpgrp(STDIN_FILENO));

	for (i = 1; i < argc; i++) {
		if ((cpid = fork()) == -1)
			errmsg_exit1("fork failed, %s\n", ERR_MSG);

		if (cpid == 0) {
			if (argv[i][0] == 'd')	/* 'd' --> to different pgrp */
				if (setpgid(0, 0) == -1)
					errmsg_exit1("setpgid failed, %s\n",
						ERR_MSG);

			sigemptyset(&sa.sa_mask);
			sa.sa_flags = 0;
			sa.sa_handler = sig_handler;
			if (sigaction(SIGHUP, &sa, NULL) == -1)
				errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

			break;	/* Child process exits loop */
		}
	}

	/* All processes fall through to here */

	alarm(30);	/* Ensure each process eventually terminates */

	printf("PID=%d, PGID=%d\n", getpid(), getpgrp());
	while (1)
		pause();
}

static void
sig_handler(int sig)
{
	printf("PID %d: caught signal %d (%s)\n", getpid(), sig,
		strsignal(sig));
}
