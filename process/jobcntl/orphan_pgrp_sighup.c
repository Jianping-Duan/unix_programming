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
#include <libgen.h>

static void sig_handler(int);

int
main(int argc, char *argv[])
{
	int i;
	char cmd[256];
	struct sigaction sa;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s {s(stop) | p(pause)}...\n", argv[0]);
	
	setvbuf(stdout, NULL, _IONBF, 0);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGHUP failed, %s\n", ERR_MSG);
	if (sigaction(SIGCONT, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGCONT failed, %s\n", ERR_MSG);

	printf("Parent: PID=%d, PPID=%d, PGID=%d, SID=%d\n", getpid(), getppid(),
		getpgrp(), getsid(0));

	for (i = 1; i < argc; i++)
		switch (fork()) {
			case -1:
				errmsg_exit1("fork failed, %s\n", ERR_MSG);
			case 0:
				printf("Child: PID=%d, PPID=%d, PGID=%d, SID=%d\n", getpid(),
					getppid(), getpgrp(), getsid(0));
				
				if (argv[i][0] == 's') {
					printf("PID=%d stopping...\n", getpid());
					raise(SIGSTOP);
				} else {
					alarm(120);	/* So we die if not SIGHUPed */
					printf("PID=%d pausing...\n", getpid());
					pause();
				}
			default:
				break;	/* Parent carries on round loop */
		}
	
	/* Parent falls through to here after creating all children */

	sleep(3);	/* Give children processes chance to start */
	sprintf(cmd, "ps -auxw | grep -v grep | grep %s", basename(argv[0]));
	system(cmd);
	printf("Parent process exitting.\n");
	exit(EXIT_SUCCESS);	/* And orphan them and their group */
}

static void
sig_handler(int sig)
{
	printf("PID=%d, caught signal %d (%s)\n", getpid(), sig, strsignal(sig));
}
