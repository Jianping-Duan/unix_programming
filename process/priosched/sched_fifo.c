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
#include <sys/resource.h>
#include <sys/times.h>

static void use_cpu(const char *, int, int);

int
main(void)
{
	cpu_set_t cset;
	struct rlimit rlim;
	struct sched_param sp;

	setbuf(stdout, NULL);

	/*
	 * Confine all processes to a single CPU, so that the processes won't
	 * run in parallel on multi-CPU systems.
	 */

	CPU_ZERO(&cset);
	CPU_SET(1, &cset);

	if (sched_setaffinity(getpid(), sizeof (cset), &cset) == -1)
		errmsg_exit1("sched_setaffinity failed, %s\n", ERR_MSG);

	/*
	 * Establish a CPU time limit. This demonstrates how we can ensure that
	 * a runaway realtime process is terminated if we make a programming
	 * error. The resource limit is inherited y the child created using
	 * fork().
	 *
	 * An alternative technique would be to make an alarm() call in each
	 * process (since interval timers are not inherited across fork()).
	 */

	/*
	 * Limits on the consumption of system resources by the current process
	 * and each process it creates may be obtained with the getrlimit()
	 * system call, and set with the setrlimit() system call.
	 *
	 * RLIMIT_CPU:
	 *	The maximum amount of cpu time (in seconds) to be used by each
	 *	process.
	 *
	 * more details see setrlimit(2).
	 */
	rlim.rlim_cur = 50;
	rlim.rlim_max = 50;
	if (setrlimit(RLIMIT_CPU, &rlim) == -1)
		errmsg_exit1("setrlimit - RLIMIT failed, %s\n", ERR_MSG);

	/* 
	 * Run the two processes in the lowest SCHED_FIFO priority.
	 *
	 * The sched_get_priority_max() and sched_get_priority_min() system
	 * calls return the appropriate maximum or minimum, respectively, for
	 * the scheduling policy specified by policy. The
	 * sched_rr_get_interval() system call updates the timespec structure
	 * referenced by the interval argument to contain the current execution
	 * time limit (i.e., time quantum) for the process specified by pid. If
	 * pid is zero, the current execution time limit for the calling process
	 * is returned.
	 */
	if ((sp.sched_priority = sched_get_priority_min(SCHED_FIFO)) == -1)
		errmsg_exit1("sched_get_priority_min failed, %s\n", ERR_MSG);

	if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
		errmsg_exit1("sched_setscheduler failed, %s\n", ERR_MSG);

	switch (fork()) {
		case -1:
			errmsg_exit1("fork failed, %s\n", ERR_MSG);
		case 0:
			use_cpu("Child", 5, 2);
			exit(EXIT_SUCCESS);
		default:
			use_cpu("Parent", 3, 3);
			exit(EXIT_SUCCESS);
	}
}

static void
use_cpu(const char *msg, int termsec, int yieldsec)
{
	int prevstep = 0, prevsec = 0, centisecs;
	struct tms vtms;

	while (1) {
		/*
		 * The times() function returns the value of time in CLK_TCK's
		 * of a second since the system startup time. The current value
		 * of CLK_TCK, the frequency of the statistics clock in ticks
		 * per second, may be obtained through the sysconf(3) interface.
		 */
		if (times(&vtms) == -1)
			errmsg_exit1("times failed, %s\n", ERR_MSG);

		/*
		 * tms_utime:
		 *	The CPU time charged for the execution of user
		 *	instructions.
		 *
		 * tms_stime:
		 *	The CPU time charged for execution by the system on
		 *	behalf of the process.
		 *
		 * more details see times(2).
		 */
		centisecs = (vtms.tms_utime + vtms.tms_stime) * 100 /
			sysconf(_SC_CLK_TCK);

#define CSEC_STEP 25	/* CPU centiseconds between messages */

		if (centisecs >= prevstep + CSEC_STEP) {
			prevstep += CSEC_STEP;
			printf("%s (PID %d) cpu=%0.2f\n", msg, getpid(),
				centisecs / 100.0);
		}

		if (centisecs > termsec * 100)	/* Terminate after 5 seconds */
			break;

		/* Yield once/second */
		if (centisecs >= prevsec + yieldsec * 100) {
			prevsec = centisecs;

			/*
			 * The sched_yield() system call forces the running
			 * process to relinquish the processor until it again
			 * becomes the head of its process list. It takes no
			 * arguments.
			 */
			if (sched_yield() == -1)
				fprintf(stderr, "Warning, sched_yield failed, "
					"%s\n", ERR_MSG);
		}
	}
}
