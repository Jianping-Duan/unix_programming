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
#ifndef _PTY_FORK_H_
#define _PTY_FORK_H_

#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pty_master_open.h"

#define	MAX_SNAME	512

static pid_t
pty_fork(int *mfd, char *slname, size_t snlen, const struct termios *stos,
	const struct winsize *sws)
{
	int fd, sfd, save_errno;
	pid_t cpid;
	char name[MAX_SNAME];

	if ((fd = pty_master_open(name, snlen)) == -1)
		return -1;

	if (slname != NULL) {	/* Return slave name to caller */
		if (strlen(name) < snlen)
			strncpy(slname, name, snlen);
		else {
			close(fd);
			errno = EOVERFLOW;
			return -1;
		}
	}

	if ((cpid = fork()) == -1) {
		fprintf(stderr, "fork failed, %s\n", ERR_MSG);
		save_errno = errno;
		close(fd);
		errno = save_errno;
		return -1;
	}

	if (cpid != 0) {	/* Parent */
		*mfd = fd;	/* Only parent gets master fd */
		return cpid;	/* Like parent of fork() */
	}

	/* Child process starts */

	if (setsid() == -1)	/* Start a new session */
		errmsg_exit2("setsid failed, %s\n", ERR_MSG);

	close(fd);		/* Not needed in child */

	if ((sfd = open(name, O_RDWR)) == -1)	/* Becomes controlling tty */
		errmsg_exit2("open (%s) failed, %s\n", name, ERR_MSG);

	if (ioctl(sfd, TIOCSCTTY, 0) == -1)	/* Acquire controllin */
		errmsg_exit2("ioctl failed, %s\n", ERR_MSG);

	if (stos != NULL)		/* Set slave tty attributes */
		if (tcsetattr(sfd, TCSANOW, stos) == -1)
			errmsg_exit2("tcsetattr failed, %s\n", ERR_MSG);

	if (sws != NULL)		/* Set slave tty window size */
		if (tcsetwinsize(sfd, sws) == -1)
			errmsg_exit2("tcsetwinsize failed, %s\n", ERR_MSG);

	/* Duplicate pty slave to be child's stdin, stdout, and stderr */

	if (dup2(sfd, STDIN_FILENO) != STDIN_FILENO)
		errmsg_exit2("dup2 - STDIN_FILENO failed, %s\n", ERR_MSG);
	if (dup2(sfd, STDOUT_FILENO) != STDOUT_FILENO)
		errmsg_exit2("dup2 - STDOUT_FILENO failed, %s\n", ERR_MSG);
	if (dup2(sfd, STDERR_FILENO) != STDERR_FILENO)
		errmsg_exit2("dup2 - STDERR_FILENO failed, %s\n", ERR_MSG);

	if (sfd > STDERR_FILENO)	/* Safety check */
		close(sfd);		/* No longer need this fd */

	return 0;			/* Like child of fork() */
}

#endif	/* !_PTY_FORK_H_ */
