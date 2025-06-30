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
#include <signal.h>
#include <syslog.h>
#include "fifoseqnum.h"

int
main(void)
{
	int sfd, dfd, cfd, seqnum, level;
	char clififo[CLIENT_FIFO_LEN];
	struct fifo_request req;
	struct fifo_response resp;
	mode_t perms;

	/* Create well-known FIFO, and open it for reading */

	umask(0);	/* So we get the permissions we want */
	perms = S_IRUSR | S_IWUSR | S_IWGRP;
	/*
	 * The mkfifo() system call creates a new fifo file with name path.
	 * The access permissions are specified by mode and restricted by the
	 * umask(2) of the calling process.
	 *
	 * The fifo's owner ID is set to the process's effective user ID. The
	 * fifo's group ID is set to that of the parent directory in which it is
	 * created.
	 */
	if (mkfifo(SERVER_FIFO, perms) == -1 && errno != EEXIST)
		errmsg_exit1("mkfifo failed, %s\n", ERR_MSG);
	if ((sfd = open(SERVER_FIFO, O_RDONLY)) == -1)
		errmsg_exit1("open %s for read failed, %s\n", SERVER_FIFO,
			ERR_MSG);

	/* Open an extra write descriptor, so that we never see EOF */
	if ((dfd = open(SERVER_FIFO, O_WRONLY)) == -1)
		errmsg_exit1("open %s for write failed, %s\n", SERVER_FIFO,
			ERR_MSG);

	/* Let's find out about broken client pipe via failed write() */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		errmsg_exit1("signal - SIGPIPE failed, %s\n", ERR_MSG);

	level = LOG_USER | LOG_WARNING;
	seqnum = 0;

	while (1) {
		if (read(sfd, &req, fifo_req_size) != fifo_req_size) {
			fprintf(stderr, "Error reading request; discarding\n");
			continue;
		}

		/* Open client FIFO (previously created by client) */

		snprintf(clififo, CLIENT_FIFO_LEN, CLIENT_FIFO_TEMPLATE,
			req.fr_pid);
		if ((cfd = open(clififo, O_WRONLY)) == -1) {
			fprintf(stderr, "open %s for write failed, %s\n",
				clififo, ERR_MSG);
			continue;
		}

		syslog(level, "Received request: %s, seqnum = %d", clififo,
			req.fr_seqlen);

		/* Send response and close FIFO */

		resp.fr_seqnum = seqnum;
		if (write(cfd, &resp, fifo_resp_size) != fifo_resp_size)
			fprintf(stderr, "Error writing to FIFO %s\n", clififo);

		syslog(level, "Send response: %d", seqnum);

		if (close(cfd) == -1)
			fprintf(stderr, "close %s failed, %s\n", clififo,
				ERR_MSG);

		seqnum += req.fr_seqlen;
	}

	exit(EXIT_SUCCESS);
}
