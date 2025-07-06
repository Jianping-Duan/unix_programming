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
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include "sysvmq_file.h"

static void sig_handler(int);
static void req_handler(const struct svmq_request *);

int
main(void)
{
	int svrid, flags, msgsz;
	const int loglvl = LOG_USER | LOG_WARNING;
	pid_t pid;
	struct sigaction sa;
	struct svmq_request req;

	/* Create server message queue */

	flags = O_CREAT | O_EXCL | S_IRUSR | S_IWUSR | S_IWGRP;
	if ((svrid = msgget(SVMQ_SERVER_KEY, flags)) == -1)
		errmsg_exit1("msgget failed, %s\n", ERR_MSG);

	/* Establish SIGCHLD handler to reap terminated children */

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGCHLD failed, %s\n", ERR_MSG);

	 /* Read requests, handle each in a separate child process */

	 while (1) {
		 msgsz = msgrcv(svrid, &req, SVMQ_REQ_SIZE, 0, 0);
		 if (msgsz == -1) {
			  /* Interrupted by SIGCHLD handler? */
			 if (errno == EINTR) {
				 syslog(loglvl, "msgrcv (%d) failed, "
					"interrupted by SIGCHLD handler",
					req.clientid);
				 continue;
			 }
			/* Some other error */
			syslog(loglvl, "msgrcv (%d) failed, %s", req.clientid,
				ERR_MSG);
			break;
		 }

		 if ((pid = fork()) == -1) {	/* Create child process */
			 syslog(loglvl, "fork (%d) failed, %s\n", req.clientid,
				ERR_MSG);
			 break;
		 }

		 if (pid == 0) {	/* Child handles request */
			syslog(loglvl, "Child (%d) begin handles request for "
				"client (%d)", getpid(), req.clientid);
			req_handler(&req);
			syslog(loglvl, "Child (%d) has completed the client's "
				"(%d) request handling", getpid(),
				req.clientid);
			_exit(EXIT_SUCCESS);
		 }
		
		/* Parent loops to receive next client request */
	 }

	 /* If msgrcv() or fork() fails, remove server MQ and exit */
	 removemq(svrid);

	 exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	int save_errno;

	assert(sig == SIGCHLD);
	save_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		continue;

	errno = save_errno;
}

static void
req_handler(const struct svmq_request *req)
{
	int fd;
	const int loglvl = LOG_USER | LOG_WARNING;
	ssize_t nrd;
	struct svmq_response resp;

	if ((fd = open(req->pathname, O_RDONLY)) == -1) {
		/* Open failed: send error text */
		syslog(loglvl, "Server conld't open this file %s, %s",
			req->pathname, ERR_MSG);
		resp.mtype = SVMQ_RESP_FAILURE;
		snprintf(resp.data, sizeof(resp.data),
			"Server conld't open this file");
		msgsnd(req->clientid, &resp, strlen(resp.data) + 1, 0);
		exit(EXIT_FAILURE);
	}

	/* Transmit file contents in messages with type SVMQ_RESP_DATA. */
	resp.mtype = SVMQ_RESP_DATA;
	while ((nrd = read(fd, resp.data, SVMQ_RESP_SIZE)) > 0)
		if (msgsnd(req->clientid, &resp, nrd, 0) == -1) {
			syslog(loglvl, "Server failed to send a %ld bytes "
				"response, %s", nrd, ERR_MSG);
			break;
		}

	/* Send a message of type SVMQ_RESP_END to signify end-of-file */
	resp.mtype = SVMQ_RESP_END;
	msgsnd(req->clientid, &resp, 0, 0);	/* Zero-length mtext */
}
