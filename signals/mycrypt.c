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
#include <signal.h>

#define SALT	"$1$xyz"	/* More details see crypt(3) */

static volatile int handled = 0; /* Counts number of calls to sig_handler */
static char *str2;	/* Set from argv[2] */

static void sig_handler(int);

int
main(int argc, char *argv[])
{
	const char * const salt = "$1$xyz";
	char *cstr1;
	int callnum, mismatch;
	struct sigaction sa;

	if (argc != 3)
		errmsg_exit1("Usage: %s str1 str2\n", argv[0]);

	str2 = argv[2];
	if ((cstr1 = strdup(crypt(argv[1], salt))) == NULL)
		errmsg_exit1("strdup failed, %s\n", ERR_MSG);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	callnum = 1, mismatch = 0;
	while (1) {
		if (strcmp(cstr1, crypt(argv[1], SALT)) != 0) {
			mismatch++;
			printf("Mismatch on call %d (mismatch=%d handled=%d)\n",
				callnum, mismatch, handled);
		}
		callnum++;
	}

	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	fflush(stdout);
	printf("%s, %s", strsignal(sig), crypt(str2, SALT));
	handled++;
}
