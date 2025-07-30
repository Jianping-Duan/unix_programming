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
#include <signal.h>
#include <getopt.h>

int
main(int argc, char *argv[])
{
	const char *port = DFT_PORT_NUM;
	int opt, seqnum = 0, sfd, cfd, optval = 1, reqnum, resplen;
	struct addrinfo hints, *res, *rp;
	struct sockaddr_storage caddr;
	socklen_t len = sizeof(caddr);
#define ADDRLEN	(NI_MAXHOST + NI_MAXSERV + 10)
#define BACKLOG	64
	char host[NI_MAXHOST], serv[NI_MAXSERV], addrstr[ADDRLEN];
	char req[INT_LEN], resp[INT_LEN];

	extern char *optarg;
	extern int optind;


	if (argc > 1 && strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s [-p port] [-n init-seqnum]\n", argv[0]);

	while ((opt = getopt(argc, argv, "p:n:")) != -1) {
		switch (opt) {
		case 'p':
			port = optarg;
			break;
		case 'n':
			seqnum = getint(optarg);
			break;
		default:
			errmsg_exit1("Bad options, %s\n", optarg);
		}
	}

	/* 
	 * Ignore the SIGPIPE signal, so that we find out about broken
	 * connection errors via a failure from write().
	 */

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		errmsg_exit1("signal failed, \n", ERR_MSG);

	/*
	 * Call getaddrinfo() to obtain a list of addresses that we can try
	 * binding to
	 */

	/*
	 * The getaddrinfo() function is used to get a list of addresses and
	 * port numbers for host hostname and service servname. It is a
	 * replacement for and provides more flexibility than the
	 * gethostbyname(3) and getservbyname(3) functions.
	 *
	 * The hostname and servname arguments are either pointers to
	 * NUL-terminated strings or the null pointer. An acceptable value for
	 * hostname is either a valid host name or a numeric host address string
	 * consisting of a dotted decimal IPv4 address, an IPv6 address, or a
	 * UNIX-domain address. The servname is either a decimal port number or
	 * a service name listed in services(5). At least one of hostname and
	 * servname must be non-null.
	 *
	 * hints is an optional pointer to a struct addrinfo, as defined by
	 * <netdb.h>
	 * This structure can be used to provide hints concerning the type of
	 * socket that the caller supports or wishes to use. The caller can
	 * supply the following structure elements in hints:
	 *
	 * ai_family:
	 *
	 *	The address family that should be used. When ai_family is set
	 *	to AF_UNSPEC, it means the caller will accept any address family
	 *	supported by the operating system.
	 *
	 * ai_socktype:
	 *
	 *	Denotes the type of socket that is wanted: SOCK_STREAM,
	 *	SOCK_DGRAM, SOCK_SEQPACKET, or SOCK_RAW. When ai_socktype is
	 *	zero the caller will accept any socket type.
	 *
	 * ai_protocol:
	 *
	 *	Indicates which transport protocol is desired, IPPROTO_UDP,
	 *	IPPROTO_TCP, IPPROTO_SCTP, or IPPROTO_UDPLITE. If ai_protocol
	 *	is zero the caller will accept any protocol.
	 *
	 * ai_flags:
	 *
	 *	The ai_flags field to which the hints parameter points shall be
	 *	set to zero or be the bitwise-inclusive OR of one or more of the
	 *	values AI_ADDRCONFIG, AI_ALL, AI_CANONNAME, AI_NUMERICHOST,
	 *	AI_NUMERICSERV, AI_PASSIVE and AI_V4MAPPED. For a UNIX-domain
	 *	address, ai_flags is ignored.
	 *
	 *	AI_PASSIVE:
	 *
	 *		If the AI_PASSIVE bit is set it indicates that the
	 *		returned socket address structure is intended for use in
	 *		a call to bind(2). In this case, if the hostname
	 *		argument is the null pointer, then the IP address
	 *		portion of the socket address structure will be set to
	 *		INADDR_ANY for an IPv4 address or IN6ADDR_ANY_INIT for
	 *		an IPv6 address.
	 *
	 *		If the AI_PASSIVE bit is not set, the returned socket
	 *		address structure will be ready for use in a call to
	 *		connect(2) for a connection-oriented protocol or
	 *		connect(2), sendto(2), or sendmsg(2) if a
	 *		connectionless protocol was chosen. The IP address
	 *		portion of the socket address structure will be set to
	 *		the loopback address if hostname is the null pointer
	 *		and AI_PASSIVE is not set
	 *
	 *	AI_NUMERICSERV:
	 *
	 *		If the AI_NUMERICSERV bit is set, then a non-null
	 *		servname string supplied shall be a numeric port string.
	 *		Otherwise, an EAI_NONAME error shall be returned. This
	 *		bit shall prevent any type of name resolution service
	 *		(for example, NIS+) from being invoked.
	 *
	 *	More details see getaddrinfo(3)
	 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_family = AF_INET;		/* Only allows IPv4 */
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;	/* Wildcard IP address;
						service name is numeric */
	if (getaddrinfo(NULL, port, &hints, &res) != 0)
		errmsg_exit1("getaddrinfo failed, %s\n", gai_strerror(errno));

	/*
	 * Walk through returned list until we find an address structure that
	 * can be used to successfully create and bind a socket
	 */

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1) {
			fprintf(stderr, "socket failed, %s\n", ERR_MSG);
			continue;
		}

		/*
		 * The setsockopt() system calls manipulate the options
		 * associated with a socket. Options may exist at multiple
		 * protocol levels; they are always present at the uppermost
		 * “socket” level.
		 *
		 * More details see setsockopt(2) or
		 * see [Stevens et al., 2004] Chapter 7.
		 */
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
			&optval, sizeof(optval)) == -1) {
			errmsg_exit1("setsockopt failed, %s\n", ERR_MSG);
		}

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;	/* success */
		/*  bind() failed: close this socket and try next address */
		fprintf(stderr, "bind failed, %s\n", ERR_MSG);
		close(sfd);
	}

	if (rp == NULL)
		errmsg_exit1("Could not bind socket to any address\n");

	if (listen(sfd, BACKLOG) == -1)
		errmsg_exit1("listen failed, %s\n", ERR_MSG);

	freeaddrinfo(res);

loopbegin:

	/* Accept a client connection, obtaining client's address */

	if ((cfd = accept(sfd, (struct sockaddr *)&caddr, &len)) == -1) {
		fprintf(stderr, "accept failed, %s\n", ERR_MSG);
		goto loopbegin;
	}


	/*
	 * The getnameinfo() function is used to convert a sockaddr structure to
	 * a pair of host name and service strings. It is a replacement for and
	 * provides more flexibility than the gethostbyaddr(3) and
	 * getservbyport(3) functions and is the converse of the getaddrinfo(3)
	 * function.
	 *
	 * If a link-layer address or UNIX-domain address is passed to
	 * getnameinfo(), its ASCII representation will be stored in host. The
	 * string pointed to by serv will be set to the empty string if
	 * non-NULL; flags will always be ignored. For a link-layer address,
	 * this can be used as a replacement of the legacy link_ntoa(3)
	 * function.
	 *
	 * The sockaddr structure sa should point to either a sockaddr_in,
	 * sockaddr_in6, sockaddr_dl, or sockaddr_un structure (for IPv4, IPv6,
	 * link-layer, or UNIX-domain respectively) that is salen bytes long. If
	 * salen is shorter than the length corresponding to the specified
	 * address family or longer than sizeof(struct sockaddr_storage), it
	 * returns EAI_FAMILY. Note that sa->sa_len should be consistent with
	 * salen though the value of sa->sa_len is not directly used in this
	 * function.
	 *
	 * The host and service names associated with sa are stored in host and
	 * serv which have length parameters hostlen and servlen. The maximum
	 * value for hostlen is NI_MAXHOST and the maximum value for servlen is
	 * NI_MAXSERV, as defined by <netdb.h>. If a length parameter is zero,
	 * no string will be stored. Otherwise, enough space must be provided to
	 * store the host name or service string plus a byte for the NUL
	 * terminator.
	 *
	 * About flags see getnameinfo(3)
	 *
	 */
	if (getnameinfo((struct sockaddr *)&caddr, len, host, NI_MAXHOST, serv,
		NI_MAXSERV, 0) == 0) {
		snprintf(addrstr, ADDRLEN, "(%s, %s)", host, serv);
	} else {
		snprintf(addrstr, ADDRLEN, "(?UNKNOWN?)");
	}
	printf("Connection from %s\n", addrstr);

	/* Read client request, send sequence number back */

	if (readline(cfd, req, INT_LEN) <= 0) {
		close(cfd);
		goto loopbegin;
	}

	if ((reqnum = atoi(req)) <= 0) {	/* Watch for misbehaving clients */
		close(cfd);
		goto loopbegin;
	}

	snprintf(resp, INT_LEN, "%d\n",  seqnum);
	resplen = (int)strlen(resp);
	if (write(cfd, resp, resplen) != resplen)
		fprintf(stderr, "write failed, %s\n", ERR_MSG);

	seqnum += reqnum;

	if (close(cfd) == -1)	/* close connection */
		fprintf(stderr, "close failed, %s\n", ERR_MSG);

	if (1)
		goto loopbegin;

	exit(EXIT_SUCCESS);
}
