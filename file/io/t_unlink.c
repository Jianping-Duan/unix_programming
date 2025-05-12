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
	int fd, flags;
	long i, nblk;
#define CMDLEN	256
#define BUFSIZE	1024
	char cmd[CMDLEN];
	char buf[BUFSIZE];

	if (argc < 2)
		errmsg_exit1("Usage: %s file [number-1KB-blocks]\n", argv[0]);

	nblk = argc > 2 ? getlong(argv[2], GN_GT_0) : 100000;

	/* O_EXCL so that we ensure we create a new file. */
	flags = O_RDWR | O_CREAT | O_EXCL;
	if ((fd = open(argv[1], flags, S_IRUSR | S_IWUSR)) == -1)
		errmsg_exit1("open file %s failure, %s\n", argv[1], ERR_MSG);

	if (unlink(argv[1]) == -1)	/* remove the filename */
		errmsg_exit1("unlink failure, %s\n", ERR_MSG);

	for (i = 0; i < nblk; i++)
		if (write(fd, buf, BUFSIZE) != BUFSIZE)
			errmsg_exit1("partial/failed write, %s\n", ERR_MSG);

	snprintf(cmd, CMDLEN, "df -k `dirname %s`", argv[1]);
	system(cmd);

	if (close(fd) == -1)
		errmsg_exit1("close file %s failure, %s\n", argv[1], ERR_MSG);
	printf("********** Closed file descriptor\n");

	/* 
	 * this call here should be sufficient to ensure that 
	 * the the file blocks have been freed by kernal.
	 */
	sleep(3);
	
	system(cmd);
	
	exit(EXIT_SUCCESS);
}
