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
#include <fcntl.h>
#include <getopt.h>

static void usage_info(const char *);

int
main(int argc, char *argv[])
{
	int op;
	const char *optstr = "f:b:s:S:F:";
	extern char *optarg;
	extern int optind;

	char *fname, *buf;
	int bytes, sz, osync, fun;
	int fd, oflags, nwr = 0, cwr = 0;

	if (argc != (int)strlen(optstr) + 1)
		usage_info(argv[0]);
	while ((op = getopt(argc, argv, optstr)) != -1) {
		switch (op) {
		case 'f':
			fname = optarg;
			break;
		case 'b':
			if (sscanf(optarg, "%d", &bytes) != 1)
				errmsg_exit1("Illegal number. -b %s\n", optarg);
			if (bytes <= 0)
				errmsg_exit1("Must be greater than 0, %s\n",
					optarg);
			break;
		case 's':
			if (sscanf(optarg, "%d", &sz) != 1)
				errmsg_exit1("Illegal number. -s %s\n", optarg);
			if (sz <= 0)
				errmsg_exit1("Must be greater than 0, %s\n",
					optarg);
			break;
		case 'S':
			if (sscanf(optarg, "%d", &osync) != 1)
				errmsg_exit1("Illegal number. -S %s\n", optarg);
			if (osync != 0 && osync != 1)
				errmsg_exit1("Illegal number. -S %s\n", optarg);
			break;
		case 'F':
			if (sscanf(optarg, "%d", &fun) != 1)
				errmsg_exit1("Illegal number. -F %s\n", optarg);
			switch (fun) {
			case 0:
			case 1:
			case 2:
				break;
			default:
				errmsg_exit1("Illegal number. -F %s\n", optarg);
			}
			break;
		default:
			fprintf(stderr, "Parameters error, %c\n", op);
				usage_info(argv[0]);
		}
	}
	if (optind < argc)
		usage_info(argv[0]);

	/* 
	 * open the file with the O_SYNC flag,
	 * so that all data and metadata changes are flushed to the disk.
	 */
	oflags = O_WRONLY | O_CREAT | (osync == 1 ? O_SYNC : 0);
	if ((fd = open(fname, oflags, S_IRUSR | S_IWUSR)) == -1)
		errmsg_exit1("open file %s failuer, %s.\n", fname, ERR_MSG);

	buf = xmalloc(sz);
	while (nwr < bytes) {
		cwr = MIN(sz, bytes - nwr);
		if (write(fd, buf, cwr) == -1)
			errmsg_exit1("write failure, %s.\n", ERR_MSG);

		/* 
		 * if perform an fdatasync() after each write,
		 * so that data--and possibly metadata--changes are flushed to
		 * the disk.
		 *
		 * if perform an fsync() after each write,
		 * so that data and metadata are flushed to the disk.
		 */
		switch (fun) {
		case 1:
			if (fsync(fd) == -1)
				errmsg_exit1("fsync error, %s.\n", ERR_MSG);
			break;
		case 2:
			if (fdatasync(fd) == -1)
				errmsg_exit1("fdatasync error, %s.\n", ERR_MSG);
			break;
		}
		nwr += cwr;
	}
	xfree(buf);

	if (close(fd) == -1)
		errmsg_exit1("close file %s failure, %s.\n", fname, ERR_MSG);

	return 0;
}

static void 
usage_info(const char *pname)
{
	fprintf(stderr, "Usage %s -f -b -s -S -F.\n", pname);
	fprintf(stderr, "-f: specify a file.\n");
	fprintf(stderr, "-b: the number of bytes will be write.\n");
	fprintf(stderr, "-s: the size of the buffer to be used.\n");
	fprintf(stderr, "-S: open the file with the O_SYNC flag(0 or 1).\n");
	fprintf(stderr, "-F: 0: none; 1: perform an fdatasync(); "
		"2: perform an fsync().\n");

	exit(EXIT_FAILURE);
}
