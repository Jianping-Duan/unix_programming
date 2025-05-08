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
#include <fcntl.h>

int
main(int argc, char *argv[])
{
	int fd;
	size_t len, alig;
	ssize_t nrd;
	off_t offset;
	char *buf;

	if (argc < 3)
		errmsg_exit1("Usage: %s file length [offset [aligment]]\n", argv[0]);

	len = getlong(argv[2], GN_ANY_BASE);
	offset = argc > 3 ? getlong(argv[3], GN_ANY_BASE) : 0;
	alig = argc > 4 ? getlong(argv[4], GN_ANY_BASE) : 4096;

	if ((fd = open(argv[1], O_RDONLY | O_DIRECT)) == -1)
		errmsg_exit1("open file %s failure, %s.\n", ERR_MSG);

	/*
	 * The aligned_alloc() function allocates size bytes of memory such that
     * the allocation's base address is a multiple of alignment. The requested
     * alignment must be a power of 2. Behavior is undefined if size is not an
     * integral multiple of alignment.
	 *
	 * we ensure that 'buf' is aligned on an odd multiple of
     * 'alignment'. We do this to ensure that if, for example, we ask
     * for a 256-byte aligned buffer, we don't accidentally get
     * a buffer that is also aligned on a 512-byte boundary. 
	 */
	buf = (char *)aligned_alloc(alig * 2, len + alig);
	if (buf == NULL)
		errmsg_exit1("aligned_alloc error, %s.\n", ERR_MSG);
	buf += alig;

	if (lseek(fd, offset, SEEK_SET) == -1)
		errmsg_exit1("lseek failure, %s.\n", ERR_MSG);

	if ((nrd = read(fd, buf, len)) == -1)
		errmsg_exit1("read failure, %s.\n", ERR_MSG);
	printf("Read %ld bytes.\n", nrd);
	
	return 0;
}
