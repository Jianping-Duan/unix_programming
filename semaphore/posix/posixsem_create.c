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
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

static void usage_info(const char *);

int
main(int argc, char *argv[])
{
	int opt, flags = 0;
	mode_t perms;
	unsigned int val;
	sem_t *sem;

	extern int optind;
	extern char *optarg;

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


	if (optind >= argc)
		usage_info(argv[0]);

	perms = (optind + 1 >= argc) ? S_IRUSR | S_IWUSR :
		getlong(argv[optind + 1], GN_BASE_8);
	val = (optind + 2 >= argc) ? 0 : getint(argv[optind + 2]);

	/*
	 * The sem_open() function creates or opens the named semaphore
	 * specified by name. The returned semaphore may be used in subsequent
	 * calls to sem_getvalue(3), sem_wait(3), sem_trywait(3), sem_post(3),
	 * and sem_close().
	 *
	 * This implementation places strict requirements on the value of name:
	 * it must begin with a slash (‘/’) and contain no other slash
	 * characters.
	 *
	 * If successful, the sem_open() function returns the address of the
	 * opened semaphore. If the same name argument is given to multiple
	 * calls to sem_open() by the same process without an intervening call
	 * to sem_close(), the same address is returned each time. If the
	 * semaphore cannot be opened, sem_open() returns SEM_FAILED and the
	 * global variable errno is set to indicate the error.
	 */
	if ((sem = sem_open(argv[optind], flags, perms, val)) == SEM_FAILED)
		errmsg_exit1("sem_open (%s) failed, %s\n", argv[optind],
			ERR_MSG);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname)
{
	fprintf(stderr, "Usage: %s [-cx] name [octal-perms] [value]\n", pname);
	fprintf(stderr, "\t-c\tCreate semaphore (O_CREAT)\n");
	fprintf(stderr, "\t-x\tCreate exclusively (O_EXCL)\n");

	exit(EXIT_FAILURE);
}
