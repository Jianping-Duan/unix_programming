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
	int pfd[2];
	int i, dummy;

	if (argc < 2 || strcmp(argv[0], "--help") == 0)
		errmsg_exit1("Usage: %s sleep-time...\n", argv[0]);

	setbuf(stdout, NULL);

	printf("[%s] Parent started\n", currtime("%T"));

	if (pipe(pfd) == -1)
		errmsg_exit1("pipe failed %s\n", ERR_MSG);

	for (i = 1; i < argc; i++) {
		switch (fork()) {
		case -1:
			errmsg_exit1("fork failed, %s\n", ERR_MSG);
		case 0:
			if (close(pfd[0]) == -1)
				errmsg_exit1("close pipe[0] failed, %s\n",
					ERR_MSG);

			/* 
			 * Child does some work, and lets parent know it's
			 * done
			 */
			sleep(getint(argv[i]));
			printf("[%s] child %d (PID=%d) closing pipe\n",
				currtime("%T"), i, getpid());
			if (close(pfd[1]) == -1)
				errmsg_exit1("close pipe[1] failed, %s\n",
					ERR_MSG);

			/* Child now carries on to do other things... */
			
			_exit(EXIT_SUCCESS);
		default:
			break;
		}
	}

	/* Parent comes here; close write end of pipe so we can see EOF */

	if (close(pfd[1]) == -1)
		errmsg_exit1("close pipe[1] failed, %s\n", ERR_MSG);

	/* Parent may do other work, then synchronizes with children */

	if (read(pfd[0], &dummy, sizeof(dummy)) != 0)
		errmsg_exit1("parent didn't get EOF\n");
	printf("[%s] Parent ready to go\n", currtime("%T"));

	if (wait(NULL) == -1)
		errmsg_exit1("wait failed, %s\n", ERR_MSG);

	/* Parent can now carry on to do other things... */

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
