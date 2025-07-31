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
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int
main(int argc, char *argv[])
{
	struct hostent *he;
	int i;
	char **pp, addrstr[INET6_ADDRSTRLEN];

	/*
	 * The gethostbyname() return a pointer to an object with the following
	 * structure (see gethostbyname(3)) describing an internet host
	 * referenced by name or by address, respectively.
	 *
	 * The name argument passed to gethostbyname() should point to a
	 * NUL-terminated hostname. The addr argument passed to gethostbyaddr()
	 * should point to an address which is len bytes long, in binary form
	 * (i.e., not an IP address in human readable ASCII form). The af
	 * argument specifies the address family (e.g. AF_INET, AF_INET6, etc.)
	 * of this address.
	 *
	 * The structure returned contains either the information obtained from
	 * the name server, broken-out fields from a line in /etc/hosts, or
	 * database entries supplied by the yp(8) system. The order of the
	 * lookups is controlled by the ‘hosts’ entry in nsswitch.conf(5).
	 *
	 * The members of this structure are:
	 *
	 *	h_name		Official name of the host.
	 *
	 *	h_aliases	A NULL-terminated array of alternate names for
	 *			the host.
	 *
	 *	h_addrtype	The type of address being returned; usually
	 *			AF_INET.
	 *
	 *	h_length	The length, in bytes, of the address.
	 *
	 *	h_addr_list	A NULL-terminated array of network addresses for
	 *			the host.
	 *			Host addresses are returned in network byte order.
	 *
	 *	h_addr		The first address in h_addr_list; this is for
	 *			backward compatibility.
	 *
	 * When using the nameserver, gethostbyname() will search for the named
	 * host in the current domain and its parents unless the name ends in a
	 * dot. If the name contains no dot, and if the environment variable
	 * “HOSTALIASES” contains the name of an alias file, the alias file will
	 * first be searched for an alias matching the input name. See
	 * hostname(7) for the domain search procedure and the alias file
	 * format.
	 */
	for (i = 1; i < argc; i++) {
		if ((he = gethostbyname(argv[i])) == NULL) {
			fprintf(stderr, "gethostbyname failed for %s, %s\n",
				argv[i], hstrerror(h_errno));
			continue;
		}

		printf("Canonical name: %s\n", he->h_name);

		printf("\talias(es):\t");
		for (pp = he->h_aliases; *pp != NULL; pp++)
			printf(" %s", *pp);
		printf("\n");

		printf("\tAddress type:\t");
		printf(" %s\n", (he->h_addrtype == AF_INET) ? "AF_INET" :
			(he->h_addrtype == AF_INET6) ? "AF_INET6" : "???");

		if (he->h_addrtype != AF_INET && he->h_addrtype != AF_INET6)
			continue;

		printf("\tAddress(es):\t");
		for (pp = he->h_addr_list; *pp != NULL; pp++)
			printf(" %s", inet_ntop(he->h_addrtype, *pp, addrstr,
				INET6_ADDRSTRLEN));
		printf("\n");
	}

	exit(EXIT_SUCCESS);
}
