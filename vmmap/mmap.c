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
#include <fcntl.h>

#define	MEM_SIZE	256

int
main(int argc, char *argv[])
{
	void *addr;
	int fd;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s file [new-value]\n", argv[0]);

	if ((fd = open(argv[1], O_RDWR)) == -1)
		errmsg_exit1("open file '%s' failed, %s\n", argv[1], ERR_MSG);

	addr = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
		errmsg_exit1("mmap failed, %s\n", ERR_MSG);

	if (close(fd) == -1)	/* no logger need 'fd' */
		errmsg_exit1("close failed, %s\n", ERR_MSG);

	printf("Current string=%.*s\n", MEM_SIZE, (char *)addr);

	if (argc <= 2)
		goto nonval;

	if (strlen(argv[2]) >= MEM_SIZE)
		errmsg_exit1("new-value too large\n");

	memset(addr, 0, MEM_SIZE);
	strncpy(addr, argv[2], MEM_SIZE - 1);
	/*
	 * The msync() system call writes any modified pages back to the file
	 * system and updates the file modification time. If len is 0, all
	 * modified pages within the region containing addr will be flushed; if
	 * len is non-zero, only those pages containing addr and len-1
	 * succeeding locations will be examined. The flags argument may be
	 * specified as follows:
	 *
	 * MS_ASYNC		Return immediately
	 * MS_SYNC		Perform synchronous writes
	 * MS_INVALIDATE	Invalidate all cached data
	 */
	if (msync(addr, MEM_SIZE, MS_SYNC) == -1)
		errmsg_exit1("msync failed, %s\n", ERR_MSG);

	printf("Copies \"%s\" to shared memory\n", argv[2]);

nonval:

	exit(EXIT_SUCCESS);
}
