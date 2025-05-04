/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024, 2025 Jianping Duan <static.integer@hotmail.com>
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
	int infd, outfd, oflags;
	mode_t fperms;
	ssize_t nrd;
	char buf[BUF_SIZE];

	if (argc != 3)
		errmsg_exit1("Usage: old-file new-file\n", argv[0]);

	/* opens the input file in read-only mode. */
	if ((infd = open(argv[1], O_RDONLY)) == -1)
		errmsg_exit1("open file %s failed, %s\n", argv[1], strerror(errno));

	/* 
	 * open the output file in 'rw-rw-rw-' mode.
	 * if this file already exists, truncate it.
	 * if this file not exists, new create it.
	 */
	oflags = O_CREAT | O_WRONLY | O_TRUNC;
	fperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	if ((outfd = open(argv[2], oflags, fperms)) == -1)
		errmsg_exit1("open file %s failed, %s\n", argv[1], strerror(errno));

	/* transfer data until we encounter end of input file or an error. */
	while ((nrd = read(infd, buf, BUF_SIZE)) > 0)
		if (write(outfd, buf, nrd) != nrd)
			errmsg_exit1("couldn't write whole buffer.\n");
	if (nrd == -1)
		errmsg_exit1("read failure.\n");

	/* 
	 * close intput file and output file.
	 */
	if (close(infd) == -1)
		errmsg_exit1("close input file failure, %s\n", strerror(errno));
	if (close(outfd) == -1)
		errmsg_exit1("close output file failure, %s\n", strerror(errno));
	
	return 0;
}
