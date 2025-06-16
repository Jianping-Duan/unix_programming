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
#include <sched.h>

int
main(int argc, char *argv[])
{
	int i, pol;
	struct sched_param sp;

	if (argc < 3 || strchr("rfo", argv[1][0]) == NULL)
		errmsg_exit1("Usage: %s policy priority [pid...]\n"
			"\tpolicy is 'r' (RR), 'f' (FIFO), 'o' (OTHER)\n",
			argv[0]);

	/*
	 * SCHED_FIFO:
	 *	First-in-first-out fixed priority scheduling with no round robin
	 *	scheduling;
	 *
	 * SCHED_OTHER:
	 *	 The standard time sharing scheduler;
	 *
	 * SCHED_RR:
	 *	Round-robin scheduling across same priority processes.
	 */
	pol = (argv[1][0] == 'r') ? SCHED_RR : (argv[1][0] == 'f') ?
		SCHED_FIFO : SCHED_OTHER;
	sp.sched_priority = getint(argv[2]);

	/*
	 * The sched_setscheduler() system call sets the scheduling policy and
	 * scheduling parameters of the process specified by pid to policy and
	 * the parameters specified in the sched_param structure pointed to by
	 * param, respectively. The value of the sched_priority member in the
	 * param structure must be any integer within the inclusive priority
	 * range for the scheduling policy specified by policy.
	 *
	 * In this implementation, if the value of pid is negative the system
	 * call will fail.
	 *
	 * If a process specified by pid exists and if the calling process has
	 * permission, the scheduling policy and scheduling parameters will be
	 * set for the process whose process ID is equal to pid.
	 *
	 * If pid is zero, the scheduling policy and scheduling parameters are
	 * set for the calling process.
	 *
	 * In this implementation, the policy of when a process can affect the
	 * scheduling parameters of another process is specified in IEEE Std
	 * 1003.1b-1993 (“POSIX.1b”) as a write-style operation.
	 */
	for (i = 3; i < argc; i++)
		if (sched_setscheduler(getint(argv[i]), pol, &sp) == -1)
			errmsg_exit1("sched_setpriority failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
