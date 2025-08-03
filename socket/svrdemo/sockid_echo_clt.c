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
#include <getopt.h>
#include "inetdomaintcp.h"

int
main(int argc, char *argv[])
{
	const char *host = NULL, *serv = NULL;
	int opt, sfd;
	char buf[BUF_SIZE], resp[BUF_SIZE];
	ssize_t len, nrd;

	extern char *optarg;
	extern int optind;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s -h host-addr -s service\n", argv[0]);

	while ((opt = getopt(argc, argv, "h:s:")) != -1) {
		switch (opt) {
		case 'h':
			host = optarg;
			break;
		case 's':
			serv = optarg;
			break;
		default:
			errmsg_exit1("Bad options\n");
		}
	}

	if (host == NULL && serv == NULL)
		errmsg_exit1("Must specify -h and -s arguments\n");

	if ((sfd = idtcp4_connect(host, serv)) == -1)
		errmsg_exit1("Could not connect to server\n");

	while (1) {
		printf("Message-> ");
		fflush(stdout);

		if (fgets(buf, BUF_SIZE, stdin) == NULL)
			continue;

		if ((len = strlen(buf)) <= 1)	/* only "\n" */
			continue;

		if (buf[len - 1] == '\n') {
			buf[len - 1] = '\0';	/* erase '\n' */
			len--;
		}

		if (write(sfd, buf, len) != len)
			errmsg_exit1("write failed, %s\n", ERR_MSG);

		if ((nrd = read(sfd, resp, BUF_SIZE)) == -1)
			errmsg_exit1("read failed, %s\n", ERR_MSG);

		printf(">>> Server response %.*s\n", (int)nrd, resp);
	}

	exit(EXIT_SUCCESS);
}
