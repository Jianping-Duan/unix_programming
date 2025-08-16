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
/*
 * Employ the self-pipe trick so that we can avoid race conditions while both
 * selecting on a set of file descriptors and also waiting for a signal.
 */
#include "unibsd.h"
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

static int pfd[2];	/* File descriptors for pipe */

static void sig_handler(int);

int
main(int argc, char *argv[])
{
	struct timeval timout, *pto;
	int i, fd, nfds = 0, ready, flags;
	fd_set rdfs;
	struct sigaction sa;
	char ch;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s {timeout|-} fd...\n"
			"\t\t('-' means infinite timeout)\n", argv[0]);

	if (strcmp(argv[1], "-") == 0)
		pto = NULL;
	else {
		timout.tv_sec = getlong(argv[1], 0);
		timout.tv_usec = 0;	/* No microseconds */
		pto = &timout;
	}

	/* Build the 'rdfs' from the fd given in command line */

	FD_ZERO(&rdfs);
	for (i = 2; i < argc; i++) {
		if ((fd = getint(argv[i])) > FD_SETSIZE)
			errmsg_exit1("fd exceeds limit (%d)\n", FD_SETSIZE);

		if (fd >= nfds)
			nfds = fd + 1;	/* Record maximum fd + 1 */
		FD_SET(fd, &rdfs);
	}

	/* Create pipe before establishing signal handler to prevent race */

	if (pipe(pfd) == -1)
		errmsg_exit1("pipe failed, %s\n", ERR_MSG);

	FD_SET(pfd[0], &rdfs);		/* Add read end of pipe to 'rdfs' */
	nfds = MAX(nfds, pfd[0] + 1);	/* And adjust 'nfds' if required */

	/* Make read and write ends of pipe nonblocking */

	if ((flags = fcntl(pfd[0], F_GETFL)) == -1)
		errmsg_exit1("fcntl (read) - F_GETFL failed, %s\n", ERR_MSG);
	flags |= O_NONBLOCK;		/* Make read end nonblocking */
	if (fcntl(pfd[0], F_SETFL, flags) == -1)
		errmsg_exit1("fcntl (read) - F_SETFL failed, %s\n", ERR_MSG);

	if ((flags = fcntl(pfd[1], F_GETFL)) == -1)
		errmsg_exit1("fcntl (write) - F_GETFL failed, %s\n", ERR_MSG);
	flags |= O_NONBLOCK;		/* Make write end nonblocking */
	if (fcntl(pfd[1], F_SETFL, flags) == -1)
		errmsg_exit1("fcntl (write) - F_SETFL failed, %s\n", ERR_MSG);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	while ((ready = select(nfds, &rdfs, NULL, NULL, pto)) == -1 &&
		errno == EINTR)
		continue;		/* Restart if interrupted by signal */
	if (ready == -1)		/* Unexpected error */
		errmsg_exit1("select failed, %s\n", ERR_MSG);

	if (FD_ISSET(pfd[0], &rdfs)) {	/* Signale Handler was called */
		printf("A signal was caught\n");

		while (1) {		/* Consume bytes from pipe */
			if (read(pfd[0], &ch, 1) == -1) {
				if (errno == EAGAIN)
					break;	/* No more bytes */
				else
					errmsg_exit1("read failed\n");
			}
		}

		/* 
		 * Perform any actions that should be taken in response to
		 * signal
		 */
	}

	/* 
	 * Examine file descriptor sets returned by select() to see which other
	 * file descriptors are ready
	 */

	printf("Ready = %d\n", ready);
	for (i = 2; i < argc; i++) {
		fd = getint(argv[i]);
		printf("%d: %s\n", fd, FD_ISSET(fd, &rdfs) ? "r" : "");
	}

	/* And check if read end of pipe is ready */

	printf("%d: %s\t(read end of pipe)\n", pfd[0],
		FD_ISSET(pfd[0], &rdfs) ? "r" : "");

	if (pto != NULL)
		printf("timeout after select(): %ld.%03ld\n", timout.tv_sec,
			timout.tv_usec / 1000);

	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	int save_errno;

	assert(sig == SIGINT);
	save_errno = errno;
	if (write(pfd[1], "x", 1) == -1 && errno != EAGAIN)
		errmsg_exit1("write failed, %s\n", ERR_MSG);
	errno = save_errno;
}
