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
#include <sys/select.h>

static void usage_info(const char *);

int
main(int argc, char *argv[])
{
	struct timeval timout, *pto;
	fd_set rdfds, wrfds;
	int nfds = 0, fd, ready, i;
	char buf[16];

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		usage_info(argv[0]);

	/* Timeout for select() is specified in argv[1] */

	if (strcmp(argv[1], "-") == 0)
		pto = NULL;		/* Infinite timeout */
	else {
		timout.tv_sec = getlong(argv[1], 0);
		timout.tv_usec = 0;	/* No microseconds */
		pto = &timout;
	}

	/* Process remaining arguments to build file descriptor sets */

	FD_ZERO(&rdfds);
	FD_ZERO(&wrfds);

	for (i = 2; i < argc; i++) {
		if (sscanf(argv[i], "%d%2[rw]", &fd, buf) != 2)
			usage_info(argv[0]);
		if (fd >= FD_SETSIZE)
			errmsg_exit1("file descriptor exceeds limit (%d)\n",
				FD_SETSIZE);

		if (fd >= nfds)
			nfds = fd + 1;	/* Record maximum fd + 1 */
		if (strchr(buf, 'r') != NULL)
			FD_SET(fd, &rdfds);
		if (strchr(buf, 'w') != NULL)
			FD_SET(fd, &wrfds);
	}

	/*
	 * The select() system call examines the I/O descriptor sets whose
	 * addresses are passed in readfds, writefds, and exceptfds to see if
	 * some of their descriptors are ready for reading, are ready for
	 * writing, or have an exceptional condition pending, respectively.
	 * The only exceptional condition detectable is out-of-band data
	 * received on a socket. The first nfds descriptors are checked in each
	 * set; i.e., the descriptors from 0 through nfds-1 in the descriptor
	 * sets are examined. On return, select() replaces the given descriptor
	 * sets with subsets consisting of those descriptors that are ready for
	 * the requested operation. The select() system call returns the total
	 * number of ready descriptors in all the sets.
	 *
	 * The descriptor sets are stored as bit fields in arrays of integers.
	 * The following macros are provided for manipulating such descriptor
	 * sets:
	 *
	 * FD_ZERO(&fdset) initializes a descriptor set fdset to the null set.
	 * FD_SET(fd, &fdset) includes a particular descriptor fd in fdset.
	 * FD_CLR(fd, &fdset) removes fd from fdset.
	 * FD_ISSET(fd, &fdset) is non-zero if fd is a member of fdset, zero
	 * otherwise.
	 *
	 * The behavior of these macros is undefined if a descriptor value is
	 * less than zero or greater than or equal to FD_SETSIZE, which is
	 * normally at least equal to the maximum number of descriptors
	 * supported by the system.
	 *
	 * If timeout is not a null pointer, it specifies the maximum interval
	 * to wait for the selection to complete. System activity can lengthen
	 * the interval by an indeterminate amount.
	 *
	 * The select() system call returns the number of ready descriptors that
	 * are contained in the descriptor sets, or -1 if an error occurred. If
	 * the time limit expires, select() returns 0. If select() returns with
	 * an error, including one due to an interrupted system call, the
	 * descriptor sets will be unmodified.
	 */
	if ((ready = select(nfds, &rdfds, &wrfds, NULL, pto)) == -1)
		errmsg_exit1("select failed, %s\n", ERR_MSG);

	printf("Ready = %d\n", ready);
	for (fd = 0; fd < nfds; fd++)
		printf("%d: %s%s\n", fd, FD_ISSET(fd, &rdfds) ? "r" : "",
			FD_ISSET(fd, &wrfds) ? "w" : "");

	if (pto != NULL)
		printf("timeout after select(): %ld.%03ld\n", timout.tv_sec,
			timout.tv_usec / 1000);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname)
{
	fprintf(stderr, "Usage: %s {timeout|-} fd-num[rw]...\n", pname);
	fprintf(stderr, "\t- means infinite timeout\n");
	fprintf(stderr, "\tr = monitor for read\n");
	fprintf(stderr, "\tw = monitor for write\n");
	fprintf(stderr, "\te.g.: %s 0rw 1w\n", pname);

	exit(EXIT_FAILURE);
}
