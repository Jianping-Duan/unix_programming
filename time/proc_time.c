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
#include <time.h>
#include <sys/times.h>

static void show_ptime(const char *);

int
main(int argc, char *argv[])
{
	int ncs, i;

	/*
	 * The sysconf() function provides a method for applications to
	 * determine the current value of a configurable system limit or option
	 * variable. The name argument specifies the system variable to be
	 * queried. Symbolic constants for each name value are found in the
	 * include file <unistd.h>. Shell programmers who need access to these
	 * parameters should use the getconf(1) utility
	 *
	 * _SC_CLK_TCK
	 *	The frequency of the statistics clock in ticks per second.
	 */
	printf("CLOCKS_PER_SEC=%d, sysconf(_SC_CLK_TCK)=%ld\n",
		CLOCKS_PER_SEC, sysconf(_SC_CLK_TCK));
	printf("\n");

	show_ptime("At program start:\n");
	printf("\n");

	/*
	 * Call getppid() a large number of times, so that some user and system
	 * CPU time are consumed.
	 */
	ncs = (argc > 1) ? getint(argv[1]) : 30000000;
	for (i = 0; i < ncs; i++)
		(void)getppid();

	show_ptime("After getppid() loops:\n");
	printf("\n");

	exit(EXIT_SUCCESS);
}

static void
show_ptime(const char *msg)
{
	struct tms t;
	clock_t clktim;
	static long clk_tcks = 0;

	if (msg != NULL)
		printf("%s", msg);

	if (clk_tcks == 0)
		if ((clk_tcks = sysconf(_SC_CLK_TCK)) == -1)
			errmsg_exit1("sysconf faild, %s\n", ERR_MSG);

	/*
	 * The clock() function determines the amount of processor time used
	 * since the invocation of the calling process, measured in
	 * CLOCKS_PER_SECs of a second.
	 *
	 * The clock() function returns the amount of time used unless an error
	 * occurs, in which case the return value is -1.
	 */
	if ((clktim = clock()) == -1)
		errmsg_exit1("clock error\n");
	printf("\tclock returns: %d clocks-per-sec (%.2f secs)\n",
		clktim, (double)clktim / CLOCKS_PER_SEC);

	/*
	 * The times() function returns the value of time in CLK_TCK's of a
	 * second since the system startup time. The current value of CLK_TCK,
	 * the frequency of the statistics clock in ticks per second, may be
	 * obtained through the sysconf(3) interface.
	 *
	 * tms_utime	The CPU time charged for the execution of user
	 *		instructions.
	 *
	 * tms_stime	The CPU time charged for execution by the system on
	 *		behalf of the process.
	 */
	if (times(&t) == (clock_t)-1)
		errmsg_exit1("times failed, %s\n", ERR_MSG);
	printf("\ttimes yield: user CPU=%.2f, system CPU=%.2f\n",
		(double)t.tms_utime / clktim, (double)t.tms_stime / clktim);
}
