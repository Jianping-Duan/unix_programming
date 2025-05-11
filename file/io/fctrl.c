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
#include <sys/stat.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
	int fd, flags, accmode;

	if (argc != 2)
		errmsg_exit1("Usage: %s file\n", argv[0]);

	if ((fd = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR)) == -1)
		errmsg_exit1("open file %s failure, %s\n", argv[1], ERR_MSG);

	/* Get descriptor status flags */
	if ((flags = fcntl(fd, F_GETFL)) == -1)
		errmsg_exit1("fcntl getfl failure, %s\n", ERR_MSG);

	if (flags & O_SYNC)
		printf("writes are synchronized.\n");

	accmode = flags & O_ACCMODE;
	if (accmode == O_WRONLY || accmode == O_RDWR)
		printf("file is writable.\n");

	/* Set descriptor status flags */
	flags |= O_APPEND | O_DIRECT | O_SYNC;	/* O_SYNC may be ignored */
	if (fcntl(fd, F_SETFL, flags) == -1)
		errmsg_exit1("fcntl setfl failure, %s\n", ERR_MSG);

	if (close(fd) == -1)
		errmsg_exit1("close failure, %s\n", ERR_MSG);

	exit(EXIT_FAILURE);
}
