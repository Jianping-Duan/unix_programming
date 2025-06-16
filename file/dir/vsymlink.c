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

#define BUFSIZE		_POSIX_PATH_MAX

int
main(int argc, char *argv[])
{
	struct stat st;
	char buf[BUFSIZE];
	ssize_t nbytes;

	if (argc != 2)
		errmsg_exit1("Usage: %s pathname\n", argv[0]);

	/* 
	 * User lstat() to check whether the supplied pathname is a symbolic
	 * link. Alternatively, we could have checked to whether readlink()
	 * failed with EINVAL.
	 */
	if (lstat(argv[1], &st) == -1)
		errmsg_exit1("stat failed, %s\n", ERR_MSG);

	if (!S_ISLNK(st.st_mode))
		errmsg_exit1("%s is not a symbolic link.\n", argv[1]);

	if ((nbytes = readlink(argv[1], buf, BUFSIZE - 1)) == -1)
		errmsg_exit1("readlink failed, %s\n", ERR_MSG);
	buf[nbytes] = '\0';	/* Add terminating null byte */
	printf("readlink: %s --> %s\n", argv[1], buf);

	memset(buf, 0, BUFSIZE);
	if (realpath(argv[1], buf) == NULL)
		errmsg_exit1("realpath failed, %s\n", ERR_MSG);
	printf("realpath: %s --> %s\n", argv[1], buf);

	exit(EXIT_SUCCESS);
}
