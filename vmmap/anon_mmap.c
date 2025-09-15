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
#include <sys/wait.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
	int *addr, fd;

	if (argc > 1) {
		if ((fd = open("/dev/zero", O_RDWR)) == -1)
			errmsg_exit1("open '/dev/zero' failed, %s\n", ERR_MSG);
		
		addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
		if (addr == MAP_FAILED)
			errmsg_exit1("mmap failed, %s\n", ERR_MSG);

		if (close(fd) == -1)
			errmsg_exit1("close failed, %s\n", ERR_MSG);
	} else {
		addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANON, -1, 0);
		if (addr == MAP_FAILED)
			errmsg_exit1("mmap failed, %s\n", ERR_MSG);
	}
	assert(argv[0] != NULL);

	*addr = 1;		/* Initialize integer in mapped region */

	switch (fork()) {	/* Parent and child share mapping */
	case -1:
		errmsg_exit1("fork failed, %s\n", ERR_MSG);
	case 0:			/* Child: increment shared integer and exit */
		printf("Child started, value = %d\n", *addr);
		(*addr)++;
		if (munmap(addr, sizeof(int)) == -1)
			errmsg_exit1("munmap failed, %s\n", ERR_MSG);
		_exit(EXIT_SUCCESS);
	default:		/* Parent: wait for child to terminate */
		if (wait(NULL) == -1)
			errmsg_exit1("wait failed, %s\n", ERR_MSG);
		printf("In parent, value = %d\n", *addr);
		if (munmap(addr, sizeof(int)) == -1)
			errmsg_exit1("munmap failed, %s\n", ERR_MSG);
	}

	exit(EXIT_SUCCESS);
}
