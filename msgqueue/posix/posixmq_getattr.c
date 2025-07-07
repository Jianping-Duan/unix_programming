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
/*
 * NOTES
 *
 * FreeBSD implements message queue based on file descriptor. The descriptor is
 * inherited by child after fork(2). The descriptor is closed in a new image
 * after exec(3). The select(2) and kevent(2) system calls are supported for
 * message queue descriptor.
 *
 * Please see the mqueuefs(5) man page for instructions on loading the module
 * or compiling the service into the kernel.
 */
#include "unibsd.h"
#include <mqueue.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
	struct mq_attr attr;
	mqd_t mqd;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s mq-name\n", argv[0]);

	if ((mqd = mq_open(argv[1], O_RDONLY)) == (mqd_t)-1)
		errmsg_exit1("mq_open (%s) failed, %s\n", argv[1], ERR_MSG);

	if (mq_getattr(mqd, &attr) == -1)
		errmsg_exit1("mq_getattr failied, %s\n", ERR_MSG);

	printf("Maximum # of messages on queue:\t\t%ld\n", attr.mq_maxmsg);
	printf("Maximum message size:\t\t\t%ld\n", attr.mq_msgsize);
	printf("# of messages currently on queue:\t%ld\n", attr.mq_curmsgs);

	exit(EXIT_SUCCESS);
}
