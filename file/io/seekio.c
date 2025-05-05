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

int
main(int argc, char *argv[])
{
	int fd, oflags, i, j;
	mode_t fperms;
	size_t len;
	ssize_t nrd, nwr;
	char *buf;
	off_t offset;

	if (argc < 3) {
		errmsg_exit1("%s file {r<lenght>|R<length>|w<string>|s<offset>}..."
			"\n", argv[0]);
	}

	oflags = O_RDWR | O_CREAT;
	fperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	if ((fd = open(argv[1], oflags, fperms)) == -1)
		errmsg_exit1("open file '%s' error, %s.\n", argv[1], ERR_MSG);

	for (i = 2; i < argc; i++) {
		switch (argv[i][0]) {
			case 'r':	/* display bytes at current offset, as text. */
			case 'R':	/* display bytes at current offset, as hex. */
				/* 
				 * The first character of the current argument is the parameter
				 * identification.
				 * The argument of value is starting from the second character
				 * of the current argument.
				 */
				len = getlong(&argv[i][1], GN_ANY_BASE);
				buf = xmalloc(len);
				if ((nrd = read(fd, buf, len)) == -1)
					errmsg_exit1("read error, %s.\n", ERR_MSG);
				if (nrd == 0)
					printf("%s: end-of-file.\n", argv[i]);
				else {
					printf("%s: ", argv[i]);	/* display argument. */
					for (j = 0; j < nrd; j++)
						if (argv[i][0] == 'r') {
							printf("%c", isprint((unsigned char)buf[j])
								? buf[j]: '?');
						} else {
							printf("%02x ", (unsigned int)buf[j]);
						}
					printf("\n");
				}
				xfree(buf);
				break;
			case 'w':	/* write string at current offset. */
				if ((nwr = write(fd, &argv[i][1], strlen(&argv[i][1]))) == -1)
					errmsg_exit1("write error, %s.\n", ERR_MSG);
				printf("%s: wrote %ld bytes.\n", argv[i], nwr);
				break;
			case 's':	/* change file offset. */
				offset = getlong(&argv[i][1], GN_ANY_BASE);
				if (lseek(fd, offset, SEEK_SET) == -1)
					errmsg_exit1("lseek error, %s.\n", ERR_MSG);
				printf("%s: seek succeeded.\n", argv[i]);
				break;
			default:
				errmsg_exit1("Argument must start with [rRws]: %s\n", argv[i]);
		}
	}

	if (close(fd) == -1)
		errmsg_exit1("close file failure, %s\n", ERR_MSG);

	return 0;
}
