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
#include <sys/uio.h>
#include <fcntl.h>
#include <getopt.h>

static void usage_info(const char *);

int
main(int argc, char *argv[])
{
	int fd, oflags, num;
	mode_t fperms;
#define STRLEN	128
	char *fname, *str1, str2[STRLEN];
	ssize_t nwr, nrd;
#define IOVSZ	3
	struct {
		int		i_num;
		char	i_str[STRLEN];
	} dat;
	struct iovec iow[IOVSZ], ior[IOVSZ];

	int op;
	const char *optstr = "f:i:s:";

	extern char *optarg;
	extern int optind;

	if (argc != (int)strlen(optstr) + 1)
		usage_info(argv[0]);
	while ((op = getopt(argc, argv, optstr)) != -1) {
		switch (op) {
			case 'f':
				fname = optarg;
				break;
			case 'i':
				if (sscanf(optarg, "%d", &num) != 1)
					errmsg_exit1("Illegal number. -i %s\n", optarg);
				break;
			case 's':
				str1 = optarg;
				break;
			default:
				fprintf(stderr, "Parameters error.\n");
				usage_info(argv[0]);
		}
	}
	if (optind < argc)
		usage_info(argv[0]);

	/* 
	 * open file for write.
	 */
	oflags = O_RDWR | O_CREAT | O_TRUNC;
	fperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	if ((fd = open(fname, oflags, fperms)) == -1)
		errmsg_exit1("open file %s error, %s.\n", fname, strerror(errno));

	/* integer number */
	iow[0].iov_base = &num;
	iow[0].iov_len = sizeof(num);

	/* struct data */
	dat.i_num = num;
	strncpy(dat.i_str, str1, STRLEN);
	iow[1].iov_base = &dat;
	iow[1].iov_len = sizeof(dat);

	/* string */
	iow[2].iov_base = str1;
	iow[2].iov_len = STRLEN;

	if ((nwr = writev(fd, iow, IOVSZ)) == -1)
		errmsg_exit1("writev error, %s.\n", strerror(errno));
	printf("Wrote %ld bytes.\n", nwr);

	if (close(fd) == -1)
		errmsg_exit1("close file error, %s\n", strerror(errno));

	/*
	 * open file for read.
	 */
	oflags = O_RDONLY;
	if ((fd = open(fname, oflags)) == -1)
		errmsg_exit1("open file %s error, %s.\n", fname, strerror(errno));

	memset(&num, 0, sizeof(num));
	ior[0].iov_base = &num;
	ior[0].iov_len = sizeof(num);

	memset(&dat, 0, sizeof(dat));
	ior[1].iov_base = &dat;
	ior[1].iov_len = sizeof(dat);

	ior[2].iov_base = str2;
	ior[2].iov_len = STRLEN;

	if ((nrd = readv(fd, ior, IOVSZ)) == -1)
		errmsg_exit1("readv error, %s.\n", strerror(errno));
	printf("Readed %ld bytes.\n", nrd);
	
	printf("Integer number: %d\n", num);
	printf("Struct Data:\n");
	printf("\tinteger number: %d\n", dat.i_num);
	printf("\tstring: %s\n", dat.i_str);
	printf("String: %s\n", str2);

	if (close(fd) == -1)
		errmsg_exit1("close file error, %s\n", strerror(errno));

	exit(EXIT_SUCCESS);
}

static void 
usage_info(const char *pname)
{
	fprintf(stderr, "Usage %s -f -i -s.\n", pname);
	fprintf(stderr, "-f: specify a file.\n");
	fprintf(stderr, "-i: will write a integer number.\n");
	fprintf(stderr, "-s: will write a string.\n");
	exit(EXIT_FAILURE);
}
