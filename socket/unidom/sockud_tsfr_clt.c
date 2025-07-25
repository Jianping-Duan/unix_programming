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
#include "sockud_tsfr.h"

int
main(void)
{
	struct sockaddr_un addr;
	int sfd;
	ssize_t nrd;
	char buf[BUF_SIZE];

	/* Create client socket */

	if ((sfd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
		errmsg_exit1("socket failed, %s\n", ERR_MSG);

	/* Construct server address, and make the connection */

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = PF_UNIX;
	strncpy(addr.sun_path, SVR_SOCK_PATH, sizeof(addr.sun_path) - 1);

	/*
	 * The s argument is a socket. If it is of type SOCK_DGRAM, this call
	 * specifies the peer with which the socket is to be associated; this
	 * address is that to which datagrams are to be sent, and the only
	 * address from which datagrams are to be received. If the socket is of
	 * type SOCK_STREAM, this call attempts to make a connection to another
	 * socket. The other socket is specified by name, which is an address in
	 * the communications space of the socket. namelen indicates the amount
	 * of space pointed to by name, in bytes; the sa_len member of name is
	 * ignored. Each communications space interprets the name argument in
	 * its own way. Generally, stream sockets may successfully connect()
	 * only once; datagram sockets may use connect() multiple times to
	 * change their association. Datagram sockets may dissolve the
	 * association by connecting to an invalid address, such as a null
	 * address.
	 */
	if (connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		errmsg_exit1("connect failed, %s\n", ERR_MSG);

	/* Copy stdin to socket */
	while ((nrd = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
		if (write(sfd, buf, nrd) != nrd)
			errmsg_exit1("write partial/failed, %s\n", ERR_MSG);

	if (nrd == -1)
		errmsg_exit1("read failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
