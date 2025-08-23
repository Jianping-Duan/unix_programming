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
#ifndef _PTY_MASTER_OPEN_H_
#define _PTY_MASTER_OPEN_H_

#include <fcntl.h>

/*
 * Open a pty master, returning file descriptor, or -1 on error. On successful
 * completion, the name of the corresponding pty slave is returned in
 * 'slavename'. 'snlen' should be set to indicate the size of the buffer pointed
 * to by 'slavename'.
 */
static int
pty_master_open(char *slavename, size_t snlen)
{
	int mfd, save_errno;
	char *pn;

	/* Open pty master */

	/*
	 * The posix_openpt() function allocates a new pseudo-terminal and
	 * establishes a connection with its master device. A slave device shall
	 * be created in /dev/pts. After the pseudo-terminal has been allocated,
	 * the slave device should have the proper permissions before it can be
	 * used (see grantpt(3)). The name of the slave device can be determined
	 * by calling ptsname(3)
	 *
	 * The file status flags and file access modes of the open file
	 * description shall be set according to the value of oflag. Values for
	 * oflag are constructed by a bitwise-inclusive OR of flags from the
	 * following list, defined in <fcntl.h>:
	 *
	 * O_RDWR	Open for reading and writing.
	 *
	 * O_NOCTTY	If set posix_openpt() shall not cause the terminal
	 *		device to become the controlling terminal for the
	 *		process.
	 *
	 * O_CLOEXEC	Set the close-on-exec flag for the new file descriptor.
	 */
	if ((mfd = posix_openpt(O_RDWR | O_NOCTTY)) == -1) {
		fprintf(stderr, "posix_openpt failed, %s\n", ERR_MSG);
		return -1;
	}

	/* Grant access to slave pty */

	/*
	 * The grantpt() function is used to establish ownership and permissions
	 * of the slave device counterpart to the master device specified with
	 * fildes. The slave device's ownership is set to the real user ID of
	 * the calling process, and the permissions are set to user
	 * readable-writable and group writable. The group owner of the slave
	 * device is also set to the group “tty”.
	 */
	if (grantpt(mfd) == -1) {
		fprintf(stderr, "grantpt failed, %s\n", ERR_MSG);
		save_errno = errno;
		close(mfd);
		errno = save_errno;
		return -1;
	}

	/* Unlock slave pty */

	/*
	 * The unlockpt() function clears the lock held on the pseudo-terminal
	 * pair for the master device specified with fildes
	 */
	if (unlockpt(mfd) == -1) {
		fprintf(stderr, "unlockpt failed, %s\n", ERR_MSG);
		save_errno = errno;
		close(mfd);
		errno = save_errno;
		return -1;
	}

	/* Get slave pty name */

	/*
	 * The ptsname() function returns the full pathname of the slave device
	 * counterpart to the master device specified with fildes. This value
	 * can be used to subsequently open the appropriate slave after
	 * posix_openpt(2) and grantpt() have been called.
	 */
	if ((pn = ptsname(mfd)) == NULL) {
		fprintf(stderr, "ptsname failed, %s\n", ERR_MSG);
		save_errno = errno;
		close(mfd);
		errno = save_errno;
		return -1;
	}

	if (strlen(pn) < snlen)
		strncpy(slavename, pn, snlen);
	else {
		fprintf(stderr, "Slave name overflow\n");
		errno = EOVERFLOW;
		return -1;
	}

	return mfd;
}

#endif	/* !_PTY_MASTER_OPEN_H_ */
