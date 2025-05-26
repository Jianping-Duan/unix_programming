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

int
main(int argc, char *argv[])
{
	int signo, nsigs, sigdat, i;
	pid_t pid;
	union sigval sv;

	if (argc < 4)
		errmsg_exit1("Usage: %s pid signo sigdata [num-sigs]\n", argv[0]);

	/*
	 * Display our PID and UID, so that they can be compared with the
	 * corresponding fields of the siginfo_t argument supplied to the
	 * handler in the receiving process 
	 */
	printf("%s: PID is %d, UID is %d\n", argv[0], getpid(), getuid());

	pid = getlong(argv[1], GN_NONNEG);
	signo = getint(argv[2]);
	sigdat = getint(argv[3]);
	nsigs = (argc > 4) ? getint(argv[4]) : 1;

	/*
	 * The sigqueue() system call causes the signal specified by signo to be
	 * sent with the value specified by value to the process specified by pid.
	 * If signo is zero (the null signal), error checking is performed but no
	 * signal is actually sent. The null signal can be used to check the
	 * validity of PID.
	 *
	 * More details see sigqueue(2)
	 */
	for (i = 0; i < nsigs; i++) {
		sv.sival_int = sigdat + i;
		if (sigqueue(pid, signo, sv) == -1)
			errmsg_exit1("sigqueue failed (%d), %s\n", i, ERR_MSG);
	}

	exit(EXIT_SUCCESS);
}
