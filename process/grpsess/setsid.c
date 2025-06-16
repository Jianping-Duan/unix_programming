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
#include <fcntl.h>

int
main(void)
{
	pid_t spid;

	system("ps -p $$ -o 'pid pgid sid command'");
	printf("\n");

	if (fork() != 0)	/* Exit if parent, or on error */
		_exit(EXIT_SUCCESS);

	/*
	 ********* setsid():
	 * The setsid() system call creates a new session. The calling process
	 * is the session leader of the new session, is the process group leader
	 * of a new process group and has no controlling terminal. The calling
	 * process is the only process in either the session or the process
	 * group.
	 *
	 * Upon successful completion, the setsid() system call returns the
	 * value of the process group ID of the new process group, which is the
	 * same as the process ID of the calling process. If an error occurs,
	 * setsid() returns -1 and the global variable errno is set to indicate
	 * the error.
	 *
	 ********* getsid():
	 * The session ID of the process identified by pid is returned by
	 * getsid(). If pid is zero, getsid() returns the session ID of the
	 * current process.
	 *
	 * Upon successful completion, the getsid() system call returns the
	 * session ID of the specified process; otherwise, it returns a value of
	 * -1 and sets errno to indicate an error.
	 *
	 ******** getpgrp()/getpgid():
	 * The process group of the current process is returned by getpgrp().
	 * The process group of the process identified by pid is returned by
	 * getpgid(). If pid is zero, getpgid() returns the process group of the
	 * current process.
	 * 
	 * Process groups are used for distribution of signals, and by terminals
	 * to arbitrate requests for their input: processes that have the same
	 * process group as the terminal are foreground and may read, while
	 * others will block with a signal if they attempt to read.
	 * This system call is thus used by programs such as csh(1) to create
	 * process groups in implementing job control.  The tcgetpgrp() and
	 * tcsetpgrp() calls are used to get/set the process group of the
	 * control terminal.
	 *
	 * The getpgrp() system call always succeeds. Upon successful
	 * completion, the getpgid() system call returns the process group of
	 * the specified process; otherwise, it returns a value of -1 and sets
	 * errno to indicate the error.
	 */
	if ((spid = setsid()) == -1)
		errmsg_exit1("setsid failed, %s\n", ERR_MSG);
	printf("PID (Group ID) returned by setsid: %d\n", spid);
	printf("PID=%d, PGSID=%d, SID=%d\n", getpid(), getpgrp(), getsid(0));
	printf("\n");

	fprintf(stderr, "Following should fail, since we don't have a "
		"controlling terminal.\n");
	if (open("/dev/tty", O_RDWR) == -1)
		errmsg_exit1("open '/dev/tty' failed, %s\n", ERR_MSG);
	exit(EXIT_SUCCESS);
}
