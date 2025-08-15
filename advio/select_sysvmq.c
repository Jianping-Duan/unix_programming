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
 * NOTE: You need to use sysvmq_create and sysvmq_send for testing
 */
#include "unibsd.h"
#include <sys/msg.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <stddef.h>	/* offsetof() */

struct pbuf {
	int msqid;		/* Origin of message */
	int len;		/* Number of bytes used in mtext */
	long mtype;		/* Message type */
#define	MAX_MTEXT	512
	char mtext[MAX_MTEXT];	/* Message body */
};

static void child_mon(int, int);

int
main(int argc, char *argv[])
{
	int pfd[2], i, nfds, ready;
	fd_set rdfs;
	ssize_t nrd;
	char buf[BUF_SIZE];
	struct pbuf pmsg;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s msqid...\n", argv[0]);

	/* Create pipe used to transfer messages from children to parent */

	if (pipe(pfd) == -1)
		errmsg_exit1("pipe failed, %s\n", ERR_MSG);

	/* Create one child for each message queue being monitored */

	for (i = 1; i < argc; i++)
		switch (fork()) {
		case -1:
			fprintf(stderr, "fork failed, %s\n", ERR_MSG);
			killpg(0, SIGTERM);
			exit(EXIT_FAILURE);
		case 0:
			child_mon(getint(argv[i]), pfd[1]);
			exit(EXIT_FAILURE);
		default:
			break;
		}

unlimloops:

	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO, &rdfs);
	FD_SET(pfd[0], &rdfs);

	nfds = MAX(STDIN_FILENO, pfd[0]) + 1;

	if ((ready = select(nfds, &rdfs, NULL, NULL, NULL)) == -1)
		errmsg_exit1("select failed, %s\n", ERR_MSG);

	/* Check if terminal fd is ready */

	if (FD_ISSET(STDIN_FILENO, &rdfs)) {
		if ((nrd = read(STDIN_FILENO, buf, BUF_SIZE - 1)) == -1)
			errmsg_exit1("read (STDIN) failed, %s\n", ERR_MSG);

		buf[nrd] = '\0';
		printf("Read from terminal: %s", buf);
		if (nrd > 1 && buf[nrd - 1] != '\n')
			printf("\n");
	}

	/* Check if pipe fd is ready */

	if (FD_ISSET(pfd[0], &rdfs)) {
		nrd = read(pfd[0], &pmsg, offsetof(struct pbuf, mtext));
		if (nrd == -1)
			errmsg_exit1("read (pipe) failed, %s\n", ERR_MSG);
		if (nrd == 0)
			errmsg_exit1("EOF on pipe\n");

		if ((nrd = read(pfd[0], &pmsg.mtext, pmsg.len)) == -1)
			errmsg_exit1("read (pipe 2) failed, %s\n", ERR_MSG);
		if (nrd == 0)
			errmsg_exit1("EOF on pipe\n");

		printf("MQ %d: type=%ld; length=%d; <%.*s>\n", pmsg.msqid,
			pmsg.mtype, pmsg.len, pmsg.len, pmsg.mtext);
	}

	if (1)
		goto unlimloops;

	exit(EXIT_SUCCESS);
}

static void
child_mon(int msqid, int fd)
{
	struct pbuf pmsg;
	ssize_t mlen, wlen;

	for (;;) {
		if ((mlen = msgrcv(msqid, &pmsg.mtype, MAX_MTEXT, 0, 0)) == -1)
			errmsg_exit1("msgrcv failed, %s\n", ERR_MSG);

		pmsg.msqid = msqid;
		pmsg.len = (int)mlen;

		wlen = offsetof(struct pbuf, mtext) + mlen;

		if (write(fd, &pmsg, wlen) != wlen)
			errmsg_exit1("write failed, %s\n", ERR_MSG);
	}
}
