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
#include "sockid_ucase.h"

int
main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	unsigned short port;
	int sfd;
	ssize_t len, bytes;
	char buf[BUF_SIZE], resp[BUF_SIZE];

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s host-address [port]\n", argv[0]);
	
	port = (argc > 2) ? (unsigned short)getint(argv[2]) : DFT_PORT_NUM;

	/* Create a datagram socket; send to an address in the IPv4 domain */

	if ((sfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
		errmsg_exit1("socket failed, %s\n", ERR_MSG);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = PF_INET;
	addr.sin_port = htons(port);
	/*
	 * The inet_pton() function converts a presentation format address
	 * (that is, printable form as held in a character string) to network
	 * format (usually a struct in_addr or some other internal binary
	 * representation, in network byte order). It returns 1 if the address
	 * was valid for the specified address family, or 0 if the address was
	 * not parseable in the specified address family, or -1 if some system
	 * error occurred (in which case errno will have been set).
	 * This function is presently valid for AF_INET and AF_INET6.
	 */
	if (inet_pton(AF_INET, argv[1], &addr.sin_addr) == -1)
		errmsg_exit1("inet_pton failed for address %s\n", argv[1]);

	/* Send messages to server; echo responses on stdout */

	while (1) {
		printf("Message-> ");
		fflush(stdout);

		if (fgets(buf, BUF_SIZE, stdin) == NULL)
			break;

		if ((len = strlen(buf)) <= 1)
			continue;

		if (buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
			len--;
		}

		bytes = sendto(sfd, buf, len, 0, (struct sockaddr *)&addr,
			sizeof(addr));
		if (bytes != len) {
			fprintf(stderr, "sendto failed, %s\n", ERR_MSG);
			continue;
		}

		bytes = recvfrom(sfd, resp, BUF_SIZE, 0, NULL, NULL);
		if (bytes == -1) {
			fprintf(stderr, "recvfrom failed, %s\n", ERR_MSG);
			continue;
		}

		printf(">>> Server response %.*s\n", (int)bytes, resp);
	}

	exit(EXIT_SUCCESS);
}
