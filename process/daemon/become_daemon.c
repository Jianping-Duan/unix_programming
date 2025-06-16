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
#include <sys/stat.h>
#include <fcntl.h>

#define	BD_NO_CHDIR		01	/* Don't chdir('/') */
#define BD_NO_CLOSE_FILES	02	/* Don't close all open files */
#define BD_NO_REOPEN_STDFDS	04	/* Don't reopen stdin, stdout and stderr
					   to '/dev/null' */
#define BD_NO_UMASK0		010	/* Don't do a umask(0) */
#define BD_MAX_CLOSE	8192	/* Maximum file descriptors to close if
				   sysconf(_SC_OPEN_MAX) is indeterminate */

static int become_daemon(int);

int
main(int argc, char *argv[])
{
#define FNAME_LEN		64
#define PROC_INFO_LEN	512
	int fd;
	char fname[FNAME_LEN] = "/tmp";
	char pinfo[PROC_INFO_LEN];
	int flags;
	mode_t fperms;

	become_daemon(0);

	strncat(fname, strrchr(argv[0], '/'), FNAME_LEN);
	strncat(fname, ".pinfo", FNAME_LEN);
	flags = O_CREAT | O_TRUNC | O_RDWR;
	fperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	if ((fd = open(fname, flags, fperms)) == -1)
		exit(EXIT_FAILURE);

	snprintf(pinfo, PROC_INFO_LEN, "%5d %5d %5d %5d %10s %31s\n", getpid(),
		getppid(), getpgrp(), getsid(0), ttyname(fd), argv[0]);
	if (write(fd, pinfo, strlen(pinfo)) <= 0)
		exit(EXIT_FAILURE);

	/* Normally a daemon would live forever; we just sleep for a while */
	sleep((argc > 1) ? getint(argv[1]) : 30);
	
	exit(EXIT_SUCCESS);
}

static int
become_daemon(int flags)
{
	int maxfd, fd;

	/* Become background process */
	switch(fork()) {
	case -1: return -1;
	case 0: break;			/* Child falls through...*/
	default: _exit(EXIT_SUCCESS);	/* while parent terminates */
	}

	/* Become leader of new session */
	if (setsid() == -1)
		return -2;

	/* Ensure we are not session leader */
	switch(fork()) {
	case -1: return -1;
	case 0: break;			/* Child falls through...*/
	default: _exit(EXIT_SUCCESS);	/* while parent terminates */
	}

	if (!(flags & BD_NO_UMASK0))
		umask(0);	/* Clear file mode creation mask */

	if (!(flags & BD_NO_CHDIR))
		chdir("/");	/* Change to root directory */

	if (!(flags & BD_NO_CLOSE_FILES)) {	/* Close all open files */
		/* Limit is indeterminate... */
		if ((maxfd = sysconf(_SC_OPEN_MAX)) == -1)
			maxfd = BD_MAX_CLOSE;	/* take a guess */
		for (fd = 0; fd < maxfd; fd++)
			close(fd);
	}

	if (!(flags & BD_NO_REOPEN_STDFDS)) {
		close(STDIN_FILENO);	/* Reopen standard fd's to /dev/null */
		
		/* 'fd' should be 0 */
		if ((fd = open("/dev/null", O_RDWR)) != STDIN_FILENO)
			return -3;

		if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
			return -4;
		if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
			return -5;
	}

	return 0;
}
