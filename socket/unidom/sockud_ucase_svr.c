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
#include "sockud_ucase.h"

int
main(void)
{
	struct sockaddr_un saddr, caddr;
	socklen_t len; 
	int sfd;
	ssize_t bytes, i;
	char buf[BUF_SIZE];

	/*
	 * SOCK_DGRAM      Datagram socket
	 *
	 * A SOCK_DGRAM socket supports datagrams (connectionless, unreliable
	 * messages of a fixed (typically small) maximum length). 
	 */
	if ((sfd = socket(PF_UNIX, SOCK_DGRAM, 0)) == -1)
		errmsg_exit1("socket failed, %s\n", ERR_MSG);

	if (strlen(SVR_SOCK_PATH) > sizeof(saddr.sun_path) - 1)
		errmsg_exit1("Server socket path to long: %s\n", SVR_SOCK_PATH);

	if (remove(SVR_SOCK_PATH) == -1 && errno != ENOENT)
		errmsg_exit1("remove (%s) failed, %s\n", SVR_SOCK_PATH,
			ERR_MSG);

	memset(&saddr, 0, sizeof(saddr));
	saddr.sun_family = PF_UNIX;
	strncpy(saddr.sun_path, SVR_SOCK_PATH, sizeof(saddr.sun_path) - 1);

	if (bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
		errmsg_exit1("bind failed, %s\n", ERR_MSG);

	/* Receive messages, convert to uppercase, and return to client */

unlimloops:

	/*
	 * The recvfrom() system calls are used to receive messages from a
	 * socket, and may be used to receive data on a socket whether or not
	 * it is connection-oriented.
	 *
	 * If from is not a null pointer and the socket is not
	 * connection-oriented, the source address of the message is filled in.
	 * The fromlen argument is a value-result argument, initialized to the
	 * size of the buffer associated with from, and modified on return to
	 * indicate the actual size of the address stored there.
	 *
	 * The recvfrom() return the length of the message on successful
	 * completion. If a message is too long to fit in the supplied buffer,
	 * excess bytes may be discarded depending on the type of socket the
	 * message is received from (see socket(2)).
	 *
	 * If no messages are available at the socket, the receive call waits
	 * for a message to arrive, unless the socket is non-blocking
	 * (see fcntl(2)) in which case the value -1 is returned and the global
	 * variable errno is set to EAGAIN.
	 */
	bytes = recvfrom(sfd, buf, BUF_SIZE, 0,
		(struct sockaddr *)&caddr, &len);
	if (bytes == -1)
		errmsg_exit1("recvfrom failed, %s\n", ERR_MSG);

	printf("Server received %ld bytes from %s\n", bytes, SVR_SOCK_PATH);

	for (i = 0; i < bytes; i++)
		buf[i] = toupper(buf[i]);

	/*
	 * The sendto() system calls are used to transmit one or more messages
	 * to another socket.
	 *
	 * The functions sendto() may be used at any time if the socket is
	 * connectionless-mode. If the socket is connection-mode, the protocol
	 * must support implied connect (currently tcp(4) is the only protocol
	 * with support) or the socket must be in a connected state before use.
	 *
	 * The address of the target is given by to with tolen specifying its
	 * size, or the equivalent msg_name and msg_namelen in struct msghdr.
	 * If the socket is in a connected state, the target address passed to
	 * sendto() is ignored. The length of the message is given by len. If
	 * the message is too long to pass atomically through the underlying
	 * protocol, the error EMSGSIZE is returned, and the message is not
	 * transmitted.
	 */
	if (sendto(sfd, buf, bytes, 0, (struct sockaddr *)&caddr, len) != bytes)
		errmsg_exit1("sendto failed, %s\n", ERR_MSG);

	if (1)
		goto unlimloops;

	exit(EXIT_SUCCESS);
}
