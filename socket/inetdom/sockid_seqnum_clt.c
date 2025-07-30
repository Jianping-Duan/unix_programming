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
#include "sockid_seqnum.h"
#include <getopt.h>

int
main(int argc, char *argv[])
{
	int opt, cfd, reqlen;
	const char *host = NULL, *port = DFT_PORT_NUM, *reqnum = "1";
	char seqstr[INT_LEN];
	ssize_t nrd;
	struct addrinfo hints, *res, *rp;

	extern char *optarg;
	extern int optind;

	if (argc >= 2 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s -h host-addr [-p port] [-n seqnum]\n",
			argv[0]);

	while ((opt = getopt(argc, argv, "h:p:n:")) != -1) {
		switch (opt) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'n':
			reqnum = optarg;
			break;
		default:
			errmsg_exit1("Bad options\n");
		}
	}

	if (host == NULL)
		errmsg_exit1("Must specify -h argument\n");

	memset(&hints, 0, sizeof(hints));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_NUMERICSERV;	

	if (getaddrinfo(host, port, &hints, &res) != 0)
		errmsg_exit1("getaddrinfo failed, %s\n", gai_strerror(errno));

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (cfd == -1) {
			fprintf(stderr, "socket failed, %s\n", ERR_MSG);
			continue;
		}

		if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		/* Connect failed: close this socket and try next address */
		fprintf(stderr, "connect failed, %s\n", ERR_MSG);
		close(cfd);
	}

	if (rp == NULL)
		errmsg_exit1("Conld not connect socket to any address\n");

	freeaddrinfo(res);

	/* Send requested sequence number, with terminating newline */

	reqlen = (int)strlen(reqnum);
	if (write(cfd, reqnum, reqlen) != reqlen)
		errmsg_exit1("Partial/failed write (reqnum)\n");
	if (write(cfd, "\n", 1) != 1)
		errmsg_exit1("Partial/failed write (newline)\n");

	/* Read and display sequence number returned by server */

	if ((nrd = readline(cfd, seqstr, INT_LEN)) == -1)
		errmsg_exit1("readline failed\n");
	if (nrd == 0)
		errmsg_exit1("Unexpected EOF from server\n");

	printf("Sequence number: %s", seqstr);

	exit(EXIT_SUCCESS);
}
