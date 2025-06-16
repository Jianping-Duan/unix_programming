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

	setbuf(stdout, NULL);	/* Make stdout unbuffered */

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGHUP failed, %s\n", ERR_MSG);

	if ((cpid = fork()) == -1)
		errmsg_exit1("fork failed, %s\n", ERR_MSG);

	if (cpid == 0 && argc > 1) {
		assert(argv[1] != NULL);
		/*
		 * The setpgid() system call sets the process group of the
		 * specified process pid to the specified pgrp. If pid is zero,
		 * then the call applies to the current process. If pgrp is
		 * zero, then the process id of the process specified by pid is
		 * used instead.
		 *
		 * If the affected process is not the invoking process, then it
		 * must be a child of the invoking process, it must not have
		 * performed an exec(3) operation, and both processes must be in
		 * the same session. The requested process group ID must already
		 * exist in the session of the caller, or it must be equal to
		 * the target process ID.
		 *
		 * The setpgid() function returns the value 0 if successful;
		 * otherwise the value -1 is returned and the global variable
		 * errno is set to indicate the error.
		 */
		if (setpgid(0, 0) == -1)
			errmsg_exit1("setpgid(0, 0) failed, %s\n", ERR_MSG);
	}

	printf("PID=%d, PPID=%d, PGID=%d, SID=%d\n", getpid(), getppid(),
		getpgrp(), getsid(0));
	
	/*
	 * An unhandled SIGALRM ensures this process will die if nothing else
	 * terminates it.
	 *
	 * The alarm() function sets a timer to deliver the signal SIGALRM to
	 * the calling process after the specified number of seconds. If an
	 * alarm has already been set with alarm() but has not been delivered,
	 * another call to alarm() will supersede the prior call. The request
	 * alarm(0) voids the current alarm and the signal SIGALRM will not be
	 * delivered.
	 */
	alarm(30);

	while (1) {
		pause();
		printf("%d: caught SIGHUP.\n", getpid());
	}
}

static void
sig_handler(int sig)
{
	printf("PID %d: caught %d (%s)\n", getpid(), sig, strsignal(sig));
}
