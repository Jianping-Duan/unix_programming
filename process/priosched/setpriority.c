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
#include <sys/time.h>
#include <sys/resource.h>

int
main(int argc, char *argv[])
{
	int which, who, prio;

	if (argc != 4 || strchr("pgu", argv[1][0]) == NULL) {
		errmsg_exit1("Usage: %s {p|g|u} who priority\n"
			"\tset priority of p=process; g=process group; "
			"u=processes for user\n", argv[0]);
	}

	which = (argv[1][0] == 'p') ? PRIO_PROCESS : (argv[1][0] == 'g') ?
		PRIO_PGRP : PRIO_USER;
	who = getint(argv[2]);
	prio = getint(argv[3]);

	/*
	 * The scheduling priority of the process, process group, or user, as
	 * indicated by which and who is obtained with the getpriority() system
	 * call and set with the setpriority() system call. The which argument
	 * is one of PRIO_PROCESS, PRIO_PGRP, or PRIO_USER, and who is
	 * interpreted relative to which (a process identifier for PRIO_PROCESS,
	 * process group identifier for PRIO_PGRP, and a user ID for PRIO_USER).
	 * A zero value of who denotes the current process, process group, or
	 * user. The prio argument is a value in the range -20 to 20. The
	 * default priority is 0; lower priorities cause more favorable
	 * scheduling
	 *
	 * The setpriority() function returns the value 0 if successful;
	 * otherwise the value -1 is returned and the global variable errno is
	 * set to indicate the error.
	 */
	if (setpriority(which, who, prio) == -1)
		errmsg_exit1("setpriority failed, %s\n", ERR_MSG);

	/*
	 * The getpriority() system call returns the highest priority (lowest
	 * numerical value) enjoyed by any of the specified processes. The
	 * setpriority() system call sets the priorities of all of the specified
	 * processes to the specified value. Only the super-user may lower
	 * priorities.
	 *
	 * Since getpriority() can legitimately return the value -1, it is
	 * necessary to clear the external variable errno prior to the call,
	 * then check it afterward to determine if a -1 is an error or a
	 * legitimate value.
	 */
	errno = 0;
	if ((prio = getpriority(which, who)) == -1 && errno != 0)
		errmsg_exit1("getpriority failed, %s\n", ERR_MSG);
	printf("Nice value = %d\n", prio);

	exit(EXIT_SUCCESS);
}
