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
#include "fifoseqnum.h"

static char clififo[CLIENT_FIFO_LEN];

static void remove_fifo(void);

int
main(int argc, char *argv[])
{
	int sfd, cfd;
	struct fifo_request req;
	struct fifo_response resp;
	mode_t perms;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [seq-len]\n", argv[0]);

	/* Create our FIFO (before sending request, to avoid a race) */

	umask(0);
	snprintf(clififo, CLIENT_FIFO_LEN, CLIENT_FIFO_TEMPLATE, getpid());
	perms = S_IRUSR | S_IWUSR | S_IWGRP;
	if (mkfifo(clififo, perms) == -1)
		errmsg_exit1("mkfifo %s failed, %s\n", clififo, ERR_MSG);

	if (atexit(remove_fifo) != 0)
		errmsg_exit1("atexit failed, %s\n", ERR_MSG);

	if ((sfd = open(SERVER_FIFO, O_WRONLY)) == -1)
		errmsg_exit1("open %s failed, %s\n", SERVER_FIFO, ERR_MSG);

	req.fr_pid = getpid();
	req.fr_seqlen = (argc > 1) ? getint(argv[1]) : 1;

	if (write(sfd, &req, fifo_req_size) != fifo_req_size)
		errmsg_exit1("write to %s failed, %s\n", SERVER_FIFO, ERR_MSG);

	/* Open our FIFO, read and display response */
	if ((cfd = open(clififo, O_RDONLY)) == -1)
		errmsg_exit1("open %s failed, %s\n", clififo, ERR_MSG);

	if (read(cfd, &resp, fifo_resp_size) != fifo_resp_size)
		errmsg_exit1("read from %s failed, %s\n", clififo, ERR_MSG);

	printf("Sequence Number = %d\n", resp.fr_seqnum);

	exit(EXIT_SUCCESS);
}


static void
remove_fifo(void)
{
	unlink(clififo);
}
