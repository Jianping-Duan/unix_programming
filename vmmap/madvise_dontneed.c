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

#define	MAP_SIZE	4096
#define	WRITE_SIZE	16

int
main(int argc, char *argv[])
{
	int fd, i;
	char *addr;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s file\n", argv[0]);

	setbuf(stdout, NULL);
	
	unlink(argv[1]);
	if ((fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
		errmsg_exit1("open '%s' failed, %s\n", argv[1], ERR_MSG);

	for (i = 0; i < MAP_SIZE; i++)
		if (write(fd, "x", 1) != 1)
			errmsg_exit1("write failed, %s\n", ERR_MSG);

	if (fsync(fd) == -1)
		errmsg_exit1("fsync failed, %s\n", ERR_MSG);
	if (close(fd) == -1)
		errmsg_exit1("close failed, %s\n", ERR_MSG);

	if ((fd = open(argv[1], O_RDWR)) == -1)
		errmsg_exit1("open '%s' failed, %s\n", argv[1], ERR_MSG);

	addr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
		errmsg_exit1("mmap failed, %s\n", ERR_MSG);

	printf("After mmap:\t\t");
	write(STDOUT_FILENO, addr, WRITE_SIZE);
	printf("\n");

	/*
	 * Copy-on-write semantics mean that the following modification will
	 * create private copies of the pages for this process
	 */

	for (i = 0; i < MAP_SIZE; i++)
		addr[i]++;

	printf("After modification:\t");
	write(STDOUT_FILENO, addr, WRITE_SIZE);
	printf("\n");

	/*
	 * After the following, the mapping contents revert to the original file
	 * contents.
	 *
	 * The madvise() system call allows a process that has knowledge of its
	 * memory behavior to describe it to the system. 
	 *
	 * MADV_DONTNEED	Allows the VM system to decrease the in-memory
	 *			priority of pages in the specified address
	 *			range. Consequently, future references to this
	 *			address range are more likely to incur a page
	 *			fault.
	 *
	 * Other behaviors see madvice(2).
	 */
	if (madvise(addr, MAP_SIZE, MADV_DONTNEED) == -1)
		errmsg_exit1("madvice failed, %s\n", ERR_MSG);

	printf("After MADV_DONTNEED:\t");
	write(STDOUT_FILENO, addr, WRITE_SIZE);
	printf("\n");

	exit(EXIT_SUCCESS);
}
