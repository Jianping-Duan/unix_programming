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
#include <strings.h>	/* strcasecmp() */

static int print_rlimit(const char *, int);

int
main(int argc, char *argv[])
{
	struct rlimit rlim;
	int i;
	pid_t cpid;

	if (argc < 2 || argc > 3 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s soft-limit [hard-limit]\n", argv[0]);

	print_rlimit("Initial maximum process limits:\t", RLIMIT_NPROC);

	/* 
	 * Set new process limits (hard == soft if not specified)
	 *
	 * Limits on the consumption of system resources by the current process and
	 * each process it creates may be obtained with the getrlimit() system call,
	 * and set with the setrlimit() system call.
	 */
	rlim.rlim_cur = (strcasecmp(argv[1], "inf") == 0) ? RLIM_INFINITY :
		getint(argv[1]);
	rlim.rlim_max = (argc == 2) ? rlim.rlim_cur :
		(strcasecmp(argv[2], "inf") == 0) ? RLIM_INFINITY : getint(argv[2]);
	if (setrlimit(RLIMIT_NPROC, &rlim) == -1)
		errmsg_exit1("setrlimit - RLIMIT_NPROC failed, %s\n", ERR_MSG);

	print_rlimit("New maximum process limit:\t", RLIMIT_NPROC);

	/* Create as many children as possible */

	for (i = 1; ; i++)
		switch (cpid = fork()) {
			case -1:
				errmsg_exit1("fork failed, %s\n", ERR_MSG);
			case 0:
				_exit(EXIT_SUCCESS);
			default:
				/*
				 * Parent: display message about each new child and let the
				 * resulting zombies accumulate 
				 */
				printf("Child %d (PID=%d) started\n", i, cpid);
				break;
		}
}

/*
 * The resource argument is one of the following:
 *
 * RLIMIT_AS:
 *	The maximum amount (in bytes) of virtual memory the process is allowed to
 *	map.
 *
 * RLIMIT_CORE:
 *	The largest size (in bytes) core(5) file that may be created.
 *
 * RLIMIT_CPU:
 *	The maximum amount of cpu time (in seconds) to be used by each process.
 *
 * RLIMIT_DATA:
 *	The maximum size (in bytes) of the data segment for a process; this defines
 *	how far a program may extend its break with the sbrk(2) function.
 *
 * RLIMIT_FSIZE:
 *	The largest size (in bytes) file that may be created.
 *
 * RLIMIT_KQUEUES:
 *	The maximum number of kqueues this user id is allowed to create.
 *
 * RLIMIT_MEMLOCK:
 *	The maximum size (in bytes) which a process may lock into memory using the
 *	mlock(2) system call.
 *
 * RLIMIT_NOFILE:
 *	The maximum number of open files for this process.
 *
 * RLIMIT_NPROC:
 *	The maximum number of simultaneous processes for this user id.
 *
 * RLIMIT_NPTS:
 *	The maximum number of pseudo-terminals this user id is allowed to create.
 *
 * RLIMIT_PIPEBUF:
 *	The maximum total size of in-kernel buffers for bi-directional pipes/fifos
 *	that this user id is allowed to consume. The buffers for kernel FIFOs
 *	created on the first open of a filesystem object created by (mkfifo(2))
 *	are also charged to the user ID of the process opening it, not the FIFO's
 *	filesystem owner. Despite somewhat unexpected, this is in fact fair, since
 *	user of the fifo is not necessary its creator.
 * 
 * RLIMIT_RSS:
 *	When there is memory pressure and swap is available, prioritize eviction of
 *	a process' resident pages beyond this amount (in bytes). When memory is not
 *	under pressure, this rlimit is effectively ignored. Even when there is
 *	memory pressure, the amount of available swap space and some sysctl settings
 *	like vm.swap_enabled and vm.swap_idle_enabled can affect what happens to
 *	processes that have exceeded this size.
 *
 *	Processes that exceed their set RLIMIT_RSS are not signalled or halted. The
 *	limit is merely a hint to the VM daemon to prefer to deactivate pages from
 *	processes that have exceeded their set RLIMIT_RSS.
 *
 * RLIMIT_SBSIZE:
 *	The maximum size (in bytes) of socket buffer usage for this user. This
 *	limits the amount of network memory, and hence the amount of mbufs, that
 *	this user may hold at any time.
 *
 * RLIMIT_STACK:
 *	The maximum size (in bytes) of the stack segment for a process; this defines
 *	how far a program's stack segment may be extended. Stack extension is
 *	performed automatically by the system.
 *
 * RLIMIT_SWAP:
 *	The maximum size (in bytes) of the swap space that may be reserved or used
 *	by all of this user id's processes. This limit is enforced only if bit 1
 *	of the vm.overcommit sysctl is set. Please see tuning(7) for a complete
 *	description of this sysctl.
 *
 * RLIMIT_UMTXP:
 *	The limit of the number of process-shared posix thread library objects
 *	allocated by user id.
 *
 * RLIMIT_VMEM:
 *	An alias for RLIMIT_AS.
 */
static int
print_rlimit(const char *msg, int resource)
{
	struct rlimit rlim;

	if (getrlimit(resource, &rlim) == -1) {
		fprintf(stderr, "Warning: getrlimit(%d) failed, %s\n", resource,
			ERR_MSG);
		return -1;
	}

	printf("%s soft=", msg);
	if (rlim.rlim_cur == RLIM_INFINITY)
		printf("Infinite");
	else
		printf("%ld", rlim.rlim_cur);

	printf("; hard=");
	if (rlim.rlim_max == RLIM_INFINITY)
		printf("Infinite");
	else
		printf("%ld", rlim.rlim_max);
	printf("\n");

	return 0;
}
