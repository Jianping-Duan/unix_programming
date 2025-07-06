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
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sysvmq_file.h"

static int client_id;

static void rmclimq(void);

int
main(int argc, char *argv[])
{
	struct svmq_request req;
	struct svmq_response resp;
	int svrid, msgs, fd, flags;
	ssize_t msgsz, totsz;
	mode_t perms;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s pathname [target-filename]\n", argv[0]);

	if (strlen(argv[1]) > sizeof(req.pathname) - 1)
		errmsg_exit1("pathname too long, (max: %ld bytes)\n",
			sizeof(req.pathname) - 1);

	/* Get server's queue identifier; create queue for response */

	if ((svrid = msgget(SVMQ_SERVER_KEY, S_IWUSR)) == -1)
		errmsg_exit1("msgget - Server MQ failed, %s\n", ERR_MSG);

	client_id = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | S_IWGRP);
	if (client_id == -1)
		errmsg_exit1("msgget - Client MQ failed, %s\n", ERR_MSG);

	if (atexit(rmclimq) != 0)
		errmsg_exit1("atexit failed, %s\n", ERR_MSG);

	/* Send message asking for file named in argv[1] */

	req.mtype = 1;	/* Any type will do */
	req.clientid = client_id;
	strncpy(req.pathname, argv[1], sizeof(req.pathname) - 1);
	/* Ensure string is null terminated */
	req.pathname[sizeof(req.pathname) - 1] = '\0';

	if (msgsnd(svrid, &req, SVMQ_REQ_SIZE, 0) == -1)
		errmsg_exit1("msgsnd failed, %s\n", ERR_MSG);

	/* Get first response, which may be failure notification */
	 
	if ((msgsz = msgrcv(client_id, &resp, SVMQ_RESP_SIZE, 0, 0)) == -1)
		errmsg_exit1("msgrcv (first) failed, %s\n", ERR_MSG);

	if (resp.mtype == SVMQ_RESP_FAILURE)
		errmsg_exit1("%s\n", resp.data);

	if (argc < 3)
		goto no_write_file;

	flags = O_CREAT | O_WRONLY | O_TRUNC;
	perms = S_IRUSR | S_IWUSR | S_IRGRP;
	if ((fd = open(argv[2], flags, perms)) == -1)
		errmsg_exit1("open file '%s' failed, %s\n", argv[2], ERR_MSG);

	if (write(fd, resp.data, msgsz) != msgsz)
		errmsg_exit1("Conld't write whole buffer, %s\n", ERR_MSG);

no_write_file:

	/*
	 * File was opened successfully by server; process messages
	 * (including the one already received) containing file data
	 */

	totsz = msgsz;
	msgs = 1;
	while (resp.mtype == SVMQ_RESP_DATA) {
		msgsz = msgrcv(client_id, &resp, SVMQ_RESP_SIZE, 0, 0);
		if (msgsz == -1)
			errmsg_exit1("msgrcv (%d) failed, %s\n", msgs, ERR_MSG);
		totsz += msgsz;
		msgs++;

		if (argc < 3)	/* no write file */
			continue;

		if (write(fd, resp.data, msgsz) != msgsz)
			errmsg_exit1("Conld't write whole buffer, %s\n",
				ERR_MSG);
	}

	assert(resp.mtype == SVMQ_RESP_END);
	printf("Received %ld bytes (%d messages)\n", totsz, msgs);

	if (argc >= 3) {
		if (fsync(fd) == -1)
			errmsg_exit1("Fsync failed, %s\n", ERR_MSG);
		if (close(fd) == -1)
			errmsg_exit1("Close file failed, %s\n", ERR_MSG);
	}

	exit(EXIT_SUCCESS);
}

static void
rmclimq(void)
{
	removemq(client_id);
}
