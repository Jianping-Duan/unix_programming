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

	/*
	 * The sched_getscheduler() system call returns the scheduling policy
	 * of the process specified by pid.
	 *
	 * The sched_getparam() system call will return the scheduling
	 * parameters of a process specified by pid in the sched_param structure
	 * pointed to by param.
	 *
	 * If a process specified by pid exists and if the calling process has
	 * permission, the scheduling parameters for the process whose process
	 * ID is equal to pid are returned.
	 *
	 * In this implementation, the policy of when a process can obtain the
	 * scheduling parameters of another process are detailed in IEEE Std
	 * 1003.1b-1993 (“POSIX.1b”) as a read-style operation.
	 *
	 * If pid is zero, the scheduling parameters for the calling process
	 * will be returned. In this implementation, the sched_getscheduler
	 * system call will fail if pid is negative.
	 */
	for (i = 1; i < argc; i++) {
		if ((pol = sched_getscheduler(getint(argv[i]))) == -1)
			errmsg_exit1("sched_getscheduler failed, %s\n",
				ERR_MSG);

		if (sched_getparam(getint(argv[i]), &sp) == -1)
			errmsg_exit1("sched_getparam failed, %s\n", ERR_MSG);

		printf("%s: %-5s ", argv[i], (pol == SCHED_RR) ? "RR" :
			(pol == SCHED_OTHER) ? "OTHER" : (pol == SCHED_FIFO) ?
			"FIFO" : "Unknow policy");
		printf("%2d\n", sp.sched_priority);
	}

	exit(EXIT_SUCCESS);
}
