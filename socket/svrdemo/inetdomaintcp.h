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
#ifndef _INETDOMAINTCP_H_
#define _INETDOMAINTCP_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GCC__)
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define LOGLVL		(LOG_USER | LOG_ERR)
#define ADDRSTRLEN	(NI_MAXHOST + NI_MAXSERV + 10)

static int
idtcp4_create(const char *serv, int backlog)
{
	struct addrinfo hints, *res, *rp;
	int sfd, ercode, optval = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

	if ((ercode = getaddrinfo(NULL, serv, &hints, &res)) != 0) {
		syslog(LOGLVL, "getaddrinfo failed in %s, %s", __func__,
			gai_strerror(ercode));
		return -1;
	}

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1) {
			syslog(LOGLVL, "socket failed in %s, %s", __func__,
				ERR_MSG);
			continue;
		}

		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval)) == -1) {
			syslog(LOGLVL, "setsockopt failed, %s", ERR_MSG);
			close(sfd);
			freeaddrinfo(res);
			return -1;
		}

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		/*  bind() failed: close this socket and try next address */
		syslog(LOGLVL, "bind failed in %s, %s", __func__, ERR_MSG);
		close(sfd);
	}

	if (rp == NULL) {
		syslog(LOGLVL, "Could not bind socket to any address");
		freeaddrinfo(res);
		return -1;
	}

	if (listen(sfd, backlog) == -1) {
		syslog(LOGLVL, "listen failed, %s", ERR_MSG);
		freeaddrinfo(res);
		return -1;
	}

	freeaddrinfo(res);

	return sfd;
}

static int
idtcp4_connect(const char *host, const char *serv)
{
	struct addrinfo hints, *res, *rp;
	int cfd, ercode;

	memset(&hints, 0, sizeof(hints));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_NUMERICSERV;	

	if ((ercode = getaddrinfo(host, serv, &hints, &res)) != 0) {
		syslog(LOGLVL, "getaddrinfo failed in %s, %s", __func__,
			gai_strerror(ercode));
		return -1;
	}

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (cfd == -1) {
			syslog(LOGLVL, "socket failed in %s, %s", __func__,
				ERR_MSG);
			continue;
		}

		if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		syslog(LOGLVL, "connect failed, %s", ERR_MSG);
		close(cfd);
	}

	if (rp == NULL) {
		syslog(LOGLVL, "Conld not connect socket to any address");
		freeaddrinfo(res);
		return -1;
	}

	return cfd;
}

static char *
idtcp4_addrstr(const struct sockaddr *addr, socklen_t addrlen, char *addrstr)
{
	char host[NI_MAXHOST], serv[NI_MAXSERV];

	if (getnameinfo(addr, addrlen, host, NI_MAXHOST, serv,
		NI_MAXSERV, 0) == 0) {
		snprintf(addrstr, ADDRSTRLEN, "(%s, %s)", host, serv);
	} else {
		snprintf(addrstr, ADDRSTRLEN, "(?UNKNOWN?)");
	}

	return addrstr;
}

#endif	/* !_INETDOMAINTCP_H_ */
