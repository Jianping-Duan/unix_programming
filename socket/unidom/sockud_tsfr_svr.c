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
	int sfd, cfd;
	ssize_t nrd;
	char buf[BUF_SIZE];
#define BLACKLOG	8

	/*
	 * The socket() system call creates an endpoint for communication and
	 * returns a descriptor.
	 *
	 * The domain argument specifies a communications domain within which
	 * communication will take place; this selects the protocol family which
	 * should be used. These families are defined in the include file
	 * <sys/socket.h>. 
	 *
	 *	PF_UNIX         Host-internal protocols
	 *
	 * Which has the same name except that the prefix is “AF_” in place of
	 * “PF_”.  Other protocol families may be also defined, beginning with
	 * “PF_”, with corresponding address families.
	 *
	 * The socket has the indicated type, which specifies the semantics of
	 * communication.
	 *	
	 *	SOCK_STREAM     Stream socket
	 *
	 * A SOCK_STREAM type provides sequenced, reliable, two-way connection
	 * based byte streams. An out-of-band data transmission mechanism may be
	 * supported.
	 *
	 * The protocol argument specifies a particular protocol to be used with
	 * the socket. Normally only a single protocol exists to support a
	 * particular socket type within a given protocol family. However, it is
	 * possible that many protocols may exist, in which case a particular
	 * protocol must be specified in this manner. The protocol number to use
	 * is particular to the “communication domain” in which communication is
	 * to take place; see protocols(5).
	 *
	 * The protocol argument may be set to zero (0) to request the default
	 * implementation of a socket type for the protocol, if any.
	 */
	if ((sfd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
		errmsg_exit1("socket failed, %s\n", ERR_MSG);

	if (strlen(SVR_SOCK_PATH) > sizeof(addr.sun_path) - 1)
		errmsg_exit1("Server socket path too long: %s\n",
			SVR_SOCK_PATH);

	if (remove(SVR_SOCK_PATH) == -1 && errno != ENOENT)
		errmsg_exit1("remvoe '%s' failed, %s\n", SVR_SOCK_PATH,
			ERR_MSG);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = PF_UNIX;
	strncpy(addr.sun_path, SVR_SOCK_PATH, sizeof(addr.sun_path) - 1);

	/*
	 * The bind() system call assigns the local protocol address to a
	 * socket. When a socket is created with socket(2) it exists in an
	 * address family space but has no protocol address assigned. The
	 * bind() system call requests that addr be assigned to the socket.
	 *
	 * Binding an address in the UNIX domain creates a socket in the file
	 * system that must be deleted by the caller when it is no longer needed
	 * (using unlink(2)).
	 */
	if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		errmsg_exit1("bind failed, %s\n", ERR_MSG);

	/*
	 * To accept connections, a socket is first created with socket(2), a
	 * willingness to accept incoming connections and a queue limit for
	 * incoming connections are specified with listen(), and then the
	 * connections are accepted with accept(2). The listen() system call
	 * applies only to sockets of type SOCK_STREAM or SOCK_SEQPACKET.
	 *
	 * The backlog argument defines the maximum length the queue of pending
	 * connections may grow to. The real maximum queue length will be 1.5
	 * times more than the value specified in the backlog argument. A
	 * subsequent listen() system call on the listening socket allows the
	 * caller to change the maximum queue length using a new backlog
	 * argument. If a connection request arrives with the queue full the
	 * client may receive an error with an indication of ECONNREFUSED, or,
	 * in the case of TCP, the connection will be silently dropped
	 */
	if (listen(sfd, BLACKLOG) == -1)
		errmsg_exit1("listen failed, %s\n", ERR_MSG);

unlimloops:

	/*
	 * The argument s is a socket that has been created with socket(2),
	 * bound to an address with bind(2), and is listening for connections
	 * after a listen(2). The accept() system call extracts the first
	 * connection request on the queue of pending connections, creates a
	 * new socket, and allocates a new file descriptor for the socket which
	 * inherits the state of the O_NONBLOCK and O_ASYNC properties and the
	 * destination of SIGIO and SIGURG signals from the original socket s.
	 *
	 * If no pending connections are present on the queue, and the original
	 * socket is not marked as non-blocking, accept() blocks the caller
	 * until a connection is present. If the original socket is marked
	 * non-blocking and no pending connections are present on the queue,
	 * accept() returns an error as described below. The accepted socket may
	 * not be used to accept more connections. The original socket s remains
	 * open.
	 *
	 * The argument addr is a result argument that is filled-in with the
	 * address of the connecting entity, as known to the communications
	 * layer. The exact format of the addr argument is determined by the
	 * domain in which the communication is occurring. A null pointer may
	 * be specified for addr if the address information is not desired; in
	 * this case, addrlen is not used and should also be null. Otherwise,
	 * the addrlen argument is a value-result argument; it should initially
	 * contain the amount of space pointed to by addr; on return it will
	 * contain the actual length (in bytes) of the address returned. This
	 * call is used with connection-based socket types, currently with
	 * SOCK_STREAM
	 */
	if ((cfd = accept(sfd, NULL, NULL)) == -1)
		errmsg_exit1("accept failed, %s\n", ERR_MSG);

	while ((nrd = read(cfd, buf, BUF_SIZE)) > 0)
		if (write(STDOUT_FILENO, buf, nrd) != nrd)
			errmsg_exit1("write partial/failed, %s\n", ERR_MSG);

	if (nrd == -1)
		errmsg_exit1("read failed, %s\n", ERR_MSG);

	if (close(cfd) == -1)
		errmsg_exit1("close failed, %s\n", ERR_MSG);

	if (1)
		goto unlimloops;

	exit(EXIT_SUCCESS);
}
