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
#include <sys/wait.h>
#include <time.h>	/* time(), localtime(), strftime() */

static char * currtime(const char *);

int
main(int argc, char *argv[])
{
	int i, numwaits;
	pid_t cpid;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s sleep-time...\n", argv[0]);

	for (i = 1; i < argc; i++)
		switch (cpid = fork()) {
			case -1:
				errmsg_exit1("fork failed, %s\n", ERR_MSG);
			case 0:
				printf("[%s] Child process started with PID %d, sleeping %s "
					"seconds\n", currtime("%T"), getpid(), argv[i]);
				sleep(getint(argv[i]));
				_exit(EXIT_SUCCESS);
			default:
				break;	/* Parent just continued around loop */
		}
	
	numwaits = 0;	/* Number of children so far waited for */
	while (1) {
		if ((cpid = wait(NULL)) == -1) {
			if (errno == ECHILD) {
				printf("No more children - bye\n");
				exit(EXIT_SUCCESS);
			} else {
				errmsg_exit1("wait failed, %s\n", ERR_MSG);
			}
		}

		numwaits++;
		printf("[%s] wait() returned child PID %d (numwaits=%d)\n",
			currtime("%T"), cpid, numwaits);
	}

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

	/* more formats see strftime(3) */
	sz = strftime(buf, BUFSIZE, (fmt != NULL) ? fmt : "%c", tmptr);	
	return ((sz == 0) ? NULL : buf);
}
