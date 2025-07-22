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

static void usage_info(const char *);

int
main(int argc, char *argv[])
{
	int opt, flags, fd;
	long sz;
	mode_t perms;
	void *addr;

	extern int optind;
	extern char *optarg;

	flags = O_RDWR;
	while ((opt = getopt(argc, argv, "cx")) != -1)
		switch (opt) {
		case 'c':
			flags |= O_CREAT;
			break;
		case 'x':
			flags |= O_EXCL;
			break;
		default:
			usage_info(argv[0]);
		}


	if (optind + 1 >= argc)
		usage_info(argv[0]);

	sz = getlong(argv[optind + 1], GN_ANY_BASE);
	perms = (optind + 2 >= argc) ? S_IRUSR | S_IWUSR :
		getlong(argv[optind + 1], GN_BASE_8);

	/*
	 * The shm_open() function opens (or optionally creates) a POSIX shared
	 * memory object named path. The flags argument contains a subset of
	 * the flags used by open(2). An access mode of either O_RDONLY or
	 * O_RDWR must be included in flags. The optional flags O_CREAT, O_EXCL,
	 * and O_TRUNC may also be specified.
	 *
	 * If O_CREAT is specified, then a new shared memory object named path
	 * will be created if it does not exist. In this case, the shared memory
	 * object is created with mode mode subject to the process' umask value.
	 * If both the O_CREAT and O_EXCL flags are specified and a shared
	 * memory object named path already exists, then shm_open() will fail
	 * with EEXIST.
	 *
	 * Newly created objects start off with a size of zero. If an existing
	 * shared memory object is opened with O_RDWR and the O_TRUNC flag is
	 * specified, then the shared memory object will be truncated to a size
	 * of zero. The size of the object can be adjusted via ftruncate(2) and
	 * queried via fstat(2).
	 */
	if ((fd = shm_open(argv[optind], flags, perms)) == -1)
		errmsg_exit1("shm_open (%s) failed, %s\n", argv[optind],
			ERR_MSG);

	if (ftruncate(fd, sz) == -1)
		errmsg_exit1("ftruncate failed, %s\n", ERR_MSG);

	if (sz <= 0)
		goto unmmap;

	/*
	 * The mmap() system call causes the pages starting at addr and
	 * continuing for at most len bytes to be mapped from the object
	 * described by fd, starting at byte offset offset. If len is not a
	 * multiple of the page size, the mapped region may extend past the
	 * specified range. Any such extension beyond the end of the mapped
	 * object will be zero-filled.
	 *
	 * If fd references a regular file or a shared memory object, the range
	 * of bytes starting at offset and continuing for len bytes must be
	 * legitimate for the possible (not necessarily current) offsets in the
	 * object. In particular, the offset value cannot be negative. If the
	 * object is truncated and the process later accesses a page that is
	 * wholly within the truncated region, the access is aborted and a
	 * SIGBUS signal is delivered to the process.
	 *
	 * If addr is non-zero, it is used as a hint to the system. (As a
	 * convenience to the system, the actual address of the region may
	 * differ from the address supplied.) If addr is zero, an address will
	 * be selected by the system. The actual starting address of the region
	 * is returned. A successful mmap deletes any previous mapping in the
	 * allocated address range.
	 *
	 * PROT_READ	Pages may be read.
	 * PROT_WRITE	Pages may be written.
	 *
	 * MAP_SHARED	Modifications are shared.
	 *
	 * More details see mmap(2)
	 */
	addr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
		errmsg_exit1("mmap failed, %s\n", ERR_MSG);

unmmap:

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname)
{
	fprintf(stderr, "Usage: %s [-cx] shm-name size [octal-perms]\n", pname);
	fprintf(stderr, "\t-c\tCreate semaphore (O_CREAT)\n");
	fprintf(stderr, "\t-x\tCreate exclusively (O_EXCL)\n");

	exit(EXIT_FAILURE);
}
