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
	struct sockaddr_in saddr, caddr;
	socklen_t len = sizeof(caddr);
	int sfd, i;
	ssize_t sz;
	char buf[BUF_SIZE], addrstr[INET_ADDRSTRLEN];
	const char *dstr;
	unsigned short port;

	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [port] &\n", argv[0]);

	port = (argc >= 2) ? (unsigned short)getint(argv[1]) : DFT_PORT_NUM;

	/* Create a datagram socket bound to an address in the IPv4 domain */

	if ((sfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
		errmsg_exit1("socket failed, %s\n", ERR_MSG);

	/*
	 * These routines convert 16 and 32 bit quantities between network byte
	 * order and host byte order. On machines which have a byte order which
	 * is the same as the network order, routines are defined as null
	 * macros.
	 */
	saddr.sin_family = PF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);	/* Wildcard address */
	saddr.sin_port = htons(port);

	if (bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
		errmsg_exit1("bind failed, %s\n", ERR_MSG);

	/* Receive messages, convert to uppercase, and return to client */

unlimloops:

	sz = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *)&caddr, &len);
	if (sz == -1)
		errmsg_exit1("recvfrom failed, %s\n", ERR_MSG);

	/* Display address of client that sent the message */

	/*
	 * The function inet_ntop() converts an address *src from network format
	 * (usually a struct in_addr or some other binary form, in network byte
	 * order) to presentation format (suitable for external display
	 * purposes).
	 * The size argument specifies the size, in bytes, of the buffer *dst.
	 * INET_ADDRSTRLEN and INET6_ADDRSTRLEN define the maximum size required
	 * to convert an address of the respective type. It returns NULL if a
	 * system error occurs (in which case, errno will have been set), or it
	 * returns a pointer to the destination string. This function is
	 * presently valid for AF_INET and AF_INET6.
	 */
	dstr = inet_ntop(AF_INET, &caddr.sin_addr, addrstr, INET_ADDRSTRLEN);
	if (dstr == NULL)
		fprintf(stderr, "Couldn't convert client address to string, "
			"%s\n", ERR_MSG);
	else
		fprintf(stderr, "Server received %ld bytes from (%s, %d)\n",
			sz, addrstr, ntohs(caddr.sin_port));

	for (i = 0; i < sz; i++)
		buf[i] = toupper(buf[i]);

	if (sendto(sfd, buf, sz, 0, (struct sockaddr *)&caddr, len) != sz)
		errmsg_exit1("sendto failed, %s\n", ERR_MSG);

	if (1)
		goto unlimloops;

	exit(EXIT_SUCCESS);
}
