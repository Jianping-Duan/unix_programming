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
 * 3. Neither the name of the Author nor the names of its contributors
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
	int fd1, fd2, fd3;
	int flags;
	mode_t fperms;
#define CMDLEN	1024
	char cmd[CMDLEN] = "cat ";

	if (argc != 2)
		errmsg_exit1("Usage: %s <file>\n", argv[0]);

	/* 
	 * cat <file>; echo 
	 * The echo command is used to output a newline.
	 */
	strcat(cmd, argv[1]);
	strcat(cmd, "; echo");

	/*
	 * 'fd1' and 'fd2' share same open file table entry, and thus file offset.
	 * 'fd3' has its own open file table entry, and thus a separate file offset.
	 */

	flags = O_RDWR | O_CREAT | O_TRUNC;
	fperms = S_IRUSR | S_IWUSR;
	if ((fd1 = open(argv[1], flags, fperms)) == -1)
		errmsg_exit1("open file %s failure(1), %s.\n", ERR_MSG);
	if ((fd2 = dup(fd1)) == -1)
		errmsg_exit1("dup error, %s.\n", ERR_MSG);
	if ((fd3 = open(argv[1], O_RDWR)) == -1)
		errmsg_exit1("open file %s failure(2), %s.\n", ERR_MSG);

	if (write(fd1, "hello,", 6) == -1)
		errmsg_exit1("write failure(1), %s.\n", ERR_MSG);
	system(cmd);
	if (write(fd2, " world", 6) == -1)
		errmsg_exit1("write failure(2), %s.\n", ERR_MSG);
	system(cmd);

	if (lseek(fd2, 0, SEEK_SET) == -1)
		errmsg_exit1("lseek error, %s.\n", ERR_MSG);
	if (write(fd1, "HELLO,", 6) == -1)
		errmsg_exit1("write failure(3), %s.\n", ERR_MSG);
	system(cmd);

	if (write(fd3, " WORLD", 6) == -1)
		errmsg_exit1("write failure(4), %s.\n", ERR_MSG);
	system(cmd);

	if (close(fd1) == -1)
		errmsg_exit1("close failure(1), %s.\n", ERR_MSG);
	if (close(fd2) == -1)
		errmsg_exit1("close failure(2), %s.\n", ERR_MSG);
	if (close(fd3) == -1)
		errmsg_exit1("close failure(3), %s.\n", ERR_MSG);

	return 0;
}
