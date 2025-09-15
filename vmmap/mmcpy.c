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
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
	int fdsrc, fddst;
	char *src, *dst;
	struct stat fs;

	if (argc != 3)
		errmsg_exit1("Usage: %s source-file dest-file\n", argv[0]);

	if ((fdsrc = open(argv[1], O_RDONLY)) == -1)
		errmsg_exit1("open '%s' failed, %s\n", argv[1], ERR_MSG);

	/*
	 * Use fstat() to obtain size of file: we use this to specify the size
	 * of the two mappings
	 */

	if (fstat(fdsrc, &fs) == -1)
		errmsg_exit1("fstat failed, %s\n", ERR_MSG);

	/*
	 * Handle zero-length file specially, since specifying a size of zero
	 * to mmap() will fail with the error EINVAL
	 */

	if (fs.st_size == 0)
		exit(EXIT_SUCCESS);

	src = mmap(NULL, fs.st_size, PROT_READ, MAP_SHARED, fdsrc, 0);
	if (src == MAP_FAILED)
		errmsg_exit1("mmap failed, %s\n", ERR_MSG);

	
	fddst = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fddst == -1)
		errmsg_exit1("open '%s' failed, %s\n", argv[2], ERR_MSG);

	if (ftruncate(fddst, fs.st_size) == -1)
		errmsg_exit1("ftruncate failed, %s\n", ERR_MSG);

	dst = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		fddst, 0);
	if (dst == MAP_FAILED)
		errmsg_exit1("mmap failed, %s\n", ERR_MSG);

	memcpy(dst, src, fs.st_size);	/* Copy bytes between mappings */

	if (msync(dst, fs.st_size, MS_SYNC) == -1)
		errmsg_exit1("msync failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
