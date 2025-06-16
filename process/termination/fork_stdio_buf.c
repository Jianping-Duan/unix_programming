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

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GCC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
int
main(int argc, char *argv[])
{
	pid_t pid;

	printf("This is printf function.\n");
	write(STDOUT_FILENO, "This is write function.\n", 24);

	if ((pid = fork()) == -1)
		errmsg_exit1("fork failed, %s\n", ERR_MSG);

	/*
	 * The _exit() system call terminates a process with the following
	 * consequences:
	 *
	 * All of the descriptors open in the calling process are closed. This
	 * may entail delays, for example, waiting for output to drain; a
	 * process in this state may not be killed, as it is already dying.
	 *
	 * If the parent process of the calling process has an outstanding
	 * wait(2) call or catches the SIGCHLD signal, it is notified of the
	 * calling process's termination and the status is set as defined by
	 * wait(2).
	 *
	 * The parent process-ID of all of the calling process's existing child
	 * processes are set to the process-ID of the calling process's reaper;
	 * the reaper (normally the initialization process) inherits each of
	 * these processes (see procctl(2), init(8) and the DEFINITIONS section
	 * of intro(2)).
	 *
	 * If the termination of the process causes any process group to become
	 * orphaned (usually because the parents of all members of the group
	 * have now exited; see “orphaned process group” in intro(2)), and if
	 * any member of the orphaned group is stopped, the SIGHUP signal and
	 * the SIGCONT signal are sent to all members of the newly-orphaned
	 * process group.
	 *
	 * If the process is a controlling process (see intro(2)), the SIGHUP
	 * signal is sent to the foreground process group of the controlling
	 * terminal, and all current access to the controlling terminal is
	 * revoked.
	 *
	 * Most C programs call the library routine exit(3), which flushes
	 * buffers, closes streams, unlinks temporary files, etc., before
	 * calling _exit().
	 */
	if (pid == 0 && argc > 1)
		_exit(EXIT_SUCCESS);

	exit(EXIT_SUCCESS);
}
