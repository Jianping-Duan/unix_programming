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
main(int argc, char *argv[])
{
	struct sockaddr_un saddr, caddr;
	int sfd, i;
	ssize_t len, bytes;
	char buf[BUF_SIZE];

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s msg...\n", argv[0]);

	/* Create client socket; bind to unique pathname (based on PID) */

	if ((sfd = socket(PF_UNIX, SOCK_DGRAM, 0)) == -1)
		errmsg_exit1("socket failed, %s\n", ERR_MSG);

	memset(&caddr, 0, sizeof(caddr));
	caddr.sun_family = PF_UNIX;
	snprintf(caddr.sun_path, sizeof(caddr.sun_path), CLT_SOCK_TP, getpid());

	if (bind(sfd, (struct sockaddr *)&caddr, sizeof(caddr)) == -1)
		errmsg_exit1("bind failed, %s\n", ERR_MSG);

	/* Construct address of server */

	memset(&saddr, 0, sizeof(saddr));
	saddr.sun_family = PF_UNIX;
	strncpy(saddr.sun_path, SVR_SOCK_PATH, sizeof(saddr.sun_path) - 1);

	/* Send messages to server; echo responses on stdout */

	for (i = 1; i < argc; i++) {
		len = strlen(argv[i]);
		bytes = sendto(sfd, argv[i], len, 0, (struct sockaddr *)&saddr,
				sizeof(saddr));
		if (bytes != len)
			errmsg_exit1("sendto failed, %s\n", ERR_MSG);

		bytes = recvfrom(sfd, buf, BUF_SIZE, 0, NULL, NULL);
		if (bytes == -1)
			errmsg_exit1("recvfrom failed, %s\n", ERR_MSG);

		printf("Response %d: %.*s\n", i, (int)bytes, buf);
	}

	remove(caddr.sun_path);

	exit(EXIT_SUCCESS);
}
