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
#include <sys/resource.h>
#include <sys/wait.h>

static void print_rusage(const struct rusage *);

int
main(int argc, char *argv[])
{
	pid_t cpid;
	struct rusage ru;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s command arg...\n", argv[0]);

	switch (cpid = fork()) {
	case -1:
		errmsg_exit1("fork failed, %s\n", ERR_MSG);
	case 0:
		execvp(argv[1], &argv[1]);
		errmsg_exit1("execvp failed, %s\n",ERR_MSG);
	default:
		printf("Command PID: %d\n", cpid);
		if (wait(NULL) == -1)
			errmsg_exit1("wait failed, %s\n", ERR_MSG);
		if (getrusage(RUSAGE_CHILDREN, &ru) == -1)
			errmsg_exit1("getrusage failed, %s\n", ERR_MSG);

		printf("\n");
		print_rusage(&ru);
		exit(EXIT_SUCCESS);
	}
}

static void
print_rusage(const struct rusage *ru)
{
	printf("CPU time (s):\t\t\tUser=%.3f; System=%.3f\n",
		ru->ru_utime.tv_sec + ru->ru_utime.tv_usec / 1000000.0,
		ru->ru_stime.tv_sec + ru->ru_stime.tv_usec / 1000000.0);
	printf("Max resident set size:\t\t%ld\n", ru->ru_maxrss);
	printf("Integral shared memory:\t\t%ld\n", ru->ru_ixrss);
	printf("Integral unshared data:\t\t%ld\n", ru->ru_idrss);
	printf("Integral unshared stack:\t%ld\n", ru->ru_isrss);
	printf("Page reclaims:\t\t\t%ld\n", ru->ru_minflt);
	printf("Page fault:\t\t\t%ld\n", ru->ru_majflt);
	printf("Swaps:\t\t\t\t%ld\n", ru->ru_nswap);
	printf("Block I/Os:\t\t\tinput=%ld; output=%ld\n", ru->ru_inblock,
		ru->ru_oublock);
	printf("Signals received:\t\t%ld\n", ru->ru_nsignals);
	printf("IPC message:\t\t\tsent=%ld; received=%ld\n", ru->ru_msgsnd,
		ru->ru_msgrcv);
	printf("Context switcher:\t\tvoluntary=%ld; involuntary=%ld\n",
		ru->ru_nvcsw, ru->ru_nivcsw);
}
