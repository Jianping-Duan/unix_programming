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
#include <signal.h>
#include <sys/wait.h>
#include "daemon.h"
#include "inetdomaintcp.h"

#define DFT_SERVICE	"20300"

static void sig_handler(int);
static void req_handler(int);

int
main(int argc, char *argv[])
{
	const char *serv = NULL;
	int r, sfd, cfd;
	struct sigaction sa;
	struct sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	char addrstr[ADDRSTRLEN];
#define BACKLOG	16

	serv = (argc > 1) ? argv[1] : DFT_SERVICE;

	if ((r = become_daemon(0)) != 0)
		errmsg_exit1("become_daemon failed, %d\n", r);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		syslog(LOGLVL, "sigaction failed, %s", ERR_MSG);
		exit(EXIT_FAILURE);
	}

	if ((sfd = idtcp4_create(serv, BACKLOG)) == -1) {
		syslog(LOGLVL, "idtcp4_create failed");
		exit(EXIT_FAILURE);
	}

	while (1) {
		if ((cfd = accept(sfd, (struct sockaddr *)&addr, &len)) == -1) {
			syslog(LOGLVL, "accept failed, %s", ERR_MSG);
			continue;
		}

		idtcp4_addrstr((struct sockaddr *)&addr, len, addrstr);
		syslog(LOGLVL, "Connection from %s", addrstr);

		switch (fork()) {
		case -1:
			syslog(LOGLVL, "fork failed, %s", ERR_MSG);
			close(cfd);	/* Give up on this client */
			break;		/* May be temporary; try next client */
		case 0:
			close(sfd);	/* Unneeded copy of listening socket */
			req_handler(cfd);
			break;
		default:
			close(cfd);	/* Unneeded copy of connected socket */
			break;
		}
	}

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
req_handler(int cfd)
{
	char buf[BUF_SIZE];
	ssize_t nrd;

	while ((nrd = read(cfd, buf, BUF_SIZE)) > 0)
		if (write(cfd, buf, nrd) != nrd) {
			syslog(LOGLVL, "write failed, %s", ERR_MSG);
			exit(EXIT_FAILURE);
		}

	if (nrd == -1) {
		syslog(LOGLVL, "read failed, %s", ERR_MSG);
		exit(EXIT_FAILURE);
	}
}
