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
#include <sys/file.h>	/* flock(), and lock operations. */
#include <time.h>	/* time(), localtime(), strftime() */

static char * currtime(const char *);

int
main(int argc, char *argv[])
{
	int fd, lock;
	const char *lname;

	if (argc < 3 || strchr("sx", argv[2][0]) == NULL) {
		errmsg_exit1("Usage: %s file lock [sleep-time]\n"
			"\tlock' is 's' (shared) or 'x' (exclusive)\n"
			"\t\toptionally followed by 'n' (nonblocking)\n"
			"\t'sleep-time' specifies time to hold lock\n", argv[0]);
	}

	lock = argv[2][0] == 's' ? LOCK_SH : LOCK_EX;
	lname = (lock & LOCK_SH) ? "LOCK_SH" : "LOCK_EX";
	if (argv[2][1] == 'n')
		lock |= LOCK_NB;

	if ((fd = open(argv[1], O_RDONLY)) == -1)	/* open file to be locked. */
		errmsg_exit1("open file %s failed, %s\n", argv[1], ERR_MSG);

	printf("PID %d requesting %7s at %s\n", getpid(), lname, currtime("%T"));
	if (flock(fd, lock) == -1) {
		if (errno == EWOULDBLOCK)
			errmsg_exit1("PID %d already locked.\n", getpid());
		else
			errmsg_exit1("flock (PID = %d)\n", getpid());
	}

	printf("PID %d granted %7s at %s\n", getpid(), lname, currtime("%T"));

	sleep(argc > 3 ? (int)getlong(argv[3], GN_NONNEG) : 10);
	printf("PID %d releasing %7s at %s\n", getpid(), lname, currtime("%T"));
	
	if (flock(fd, LOCK_UN) == -1)	/* unlock file */
		errmsg_exit1("flock(LOCK_UN) failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}

static char *
currtime(const char *fmt)
{
#define BUFSIZE	512
	static char buf[BUFSIZE];
	time_t tim;
	size_t sz;
	struct tm *tmptr;

	tim = time(NULL);
	if ((tmptr = localtime(&tim)) == NULL) {
		fprintf(stderr, "localtime failed, %s\n", ERR_MSG);
		return NULL;
	}

	/* more formats see man(3) */
	sz = strftime(buf, BUFSIZE, (fmt != NULL) ? fmt : "%c", tmptr);	
	return ((sz == 0) ? NULL : buf);
}
