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
	int fd;
	struct stat fs;
	char *addr;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s file\n", argv[0]);

	if ((fd = open(argv[1], O_RDONLY)) == -1)
		errmsg_exit1("open file '%s' failed, %s\n", argv[1], ERR_MSG);

	/*
	 * Obtain the size of the file and use it to specify the size of the
	 * mapping and the size of the buffer to be written 
	 */

	if (fstat(fd, &fs) == -1)
		errmsg_exit1("fstat failed, %s\n", ERR_MSG);

	/*
	 * Handle zero-length file specially, since specifying a size of zero
	 * to mmap() will fail with the error EINVAL
	 */

	if (fs.st_size == 0)
		exit(EXIT_SUCCESS);

	/*
	 * MAP_PRIVATE	Modifications are private.
	 *
	 * Upon successful completion, mmap() returns a pointer to the mapped
	 * region. Otherwise, a value of MAP_FAILED is returned and errno is set
	 * to indicate the error.
	 */
	addr = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
		errmsg_exit1("mmap failed, %s\n", ERR_MSG);

	if (write(STDOUT_FILENO, addr, fs.st_size) != fs.st_size)
		errmsg_exit1("write failed, %s\n", ERR_MSG);

	/*
	 * The munmap() system call deletes the mappings and guards for the
	 * specified address range, and causes further references to addresses
	 * within the range to generate invalid memory references.
	 */
	if (munmap(addr, fs.st_size) == -1)
		errmsg_exit1("munmap failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
