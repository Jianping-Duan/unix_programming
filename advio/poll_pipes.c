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
#include <poll.h>
#include <time.h>

int
main(int argc, char *argv[])
{
	int npip, nwr, rpip, ready, i, cnt = 0;
	struct pollfd *pfds;
	int (*fds)[2];

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s num-pipes [num-writes]\n", argv[0]);

	/*
	 * Allocate the arrays that we use. The arrays are sized according
	 * to the number of pipes specified on command line.
	 */

	assert((npip = getint(argv[1])) > 0);
	assert((nwr = (argc > 2) ? getint(argv[2]) : 1) > 0);

	pfds = xcalloc(npip, sizeof(*pfds));
	fds = xcalloc(npip, sizeof(int [2]));

	/* Create the number of pipes specified on command line */

	for (i = 0; i < npip; i++)
		if (pipe(fds[i]) == -1)
			errmsg_exit1("pipe %d failed, %s\n", i, ERR_MSG);

	/* Perform specified number of writes to random pipes */

	srandom((unsigned)time(NULL));
	for (i = 0; i < nwr; i++) {
		rpip = (int)(random() % npip);
		printf("Writing to fd: %3d (read fd: %3d)\n", fds[rpip][1],
			fds[rpip][0]);
		if (write(fds[rpip][1], "x", 1) == -1)
			errmsg_exit1("write %d failed, %s\n", fds[rpip][1],
				ERR_MSG);
	}
	printf("\n");

	/*
	 * Build the file descriptor list to be supplied to poll(). This list is
	 * set to contain the file descriptors for the read ends of all of the
	 * pipes.
	 */

	/*
	 * The fields of struct pollfd are as follows:
	 *
	 * fd		File descriptor to poll. If fd is equal to -1 then
	 *		revents is cleared (set to zero), and that pollfd is not
	 *		checked.
	 *
	 * events	Events to poll for. (See below.)
	 *
	 * revents	Events which may occur. (See below.)
	 *
	 *
	 * The event bitmasks in events and revents have the following bits:
	 *
	 * POLLIN	Data other than high priority data may be read without
	 *		blocking.
	 *
	 * POLLRDNORM	Normal data may be read without blocking.
	 *
	 * POLLRDBAND	Data with a non-zero priority may be read without
	 *		blocking.
	 *
	 * POLLPRI	High priority data may be read without blocking.
	 *
	 * POLLWRNORM	Normal data may be written without blocking.
	 *
	 * POLLWRBAND	Data with a non-zero priority may be written without
	 *		blocking.
	 *
	 * POLLERR	An exceptional condition has occurred on the device or
	 *		socket. This flag is always checked, even if not present
	 *		in the events bitmask.
	 *
	 * POLLHUP	The device or socket has been disconnected. This flag is
	 *		always checked, even if not present in the events
	 *		bitmask. Note that POLLHUP and POLLOUT should never be
	 *		present in the revents bitmask at the same time.
	 *
	 * POLLRDHUP	Remote peer closed connection, or shut down writing.
	 *		Unlike POLLHUP, POLLRDHUP must be present in the events
	 *		bitmask to be reported. Applies only to stream sockets.
	 *
	 * POLLNVAL	The file descriptor is not open, or in capability mode
	 *		the file descriptor has insufficient rights. This flag
	 *		is always checked, even if not present in the events
	 *		bitmask.
	 */
	for (i = 0; i < npip; i++) {
		pfds[i].fd = fds[i][0];		/* file descriptor to poll */
		pfds[i].events = POLLIN;	/* events to look for */
	}


	/*
	 * The poll() system call examines a set of file descriptors to see if
	 * some of them are ready for I/O. The fds argument is a pointer to an
	 * array of pollfd structures as defined in <poll.h>. The nfds argument
	 * determines the size of the fds array
	 *
	 * If timeout is neither zero nor INFTIM (-1), it specifies a maximum
	 * interval to wait for any file descriptor to become ready, in 
	 * milliseconds. If timeout is INFTIM, the poll blocks indefinitely.
	 * If timeout is zero, then poll() will return without blocking.
	 */
	if ((ready = poll(pfds, npip, 0)) == -1)
		errmsg_exit1("poll failed, %s\n", ERR_MSG);
	printf("poll() returned: %d\n", ready);

	/* Check which pipes have data available for reading */
	
	for (i = 0; i < npip; i++)
		if (pfds[i].revents & POLLIN) {
			printf("Readable: %3d\n", pfds[i].fd);
			cnt++;
		}
	assert(ready == cnt);

	xfree(pfds);
	xfree(fds);

	exit(EXIT_SUCCESS);
}
