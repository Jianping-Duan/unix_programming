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
#include <pthread.h>

#define MAX_ERROR_LEN	256

static __thread char buf[MAX_ERROR_LEN];	/* Thread-local return buffer */

static char * mystrerror(int);
static void * thread_func(void *);

int
main(void)
{
	pthread_t tid;
	int r;
	char *str;

	str = mystrerror(EINVAL);
	printf("Main thread has called mystrerror()\n");
	if ((r = pthread_create(&tid, NULL, thread_func, NULL)) != 0)
		errmsg_exit1("pthread_create failed, %d\n", r);

	if ((r = pthread_join(tid, NULL)) != 0)
		errmsg_exit1("pthread_join failed, %d\n", r);

	/*
	 * If mystrerror() is not thread-safe, then the output of this printf()
	 * be the same as that produced by the analogous printf() in
	 * thread_func()
	 */
	printf("Main thread str (%p) = %s\n", str, str);

	exit(EXIT_SUCCESS);
}

static char *
mystrerror(int eno)
{
	if (eno < 0 || eno >= sys_nerr || sys_errlist[eno] == NULL) {
		snprintf(buf, MAX_ERROR_LEN, "Unkown error %d\n", eno);
	} else {
		strncpy(buf, sys_errlist[eno], MAX_ERROR_LEN);
		buf[MAX_ERROR_LEN - 1] = '\0';
	}

	return buf;
}

static void *
thread_func(void *arg)
{
	char *str;

	assert(arg == NULL);
	printf("Other thread about to call mystrerror()\n");
	str = mystrerror(EPERM);
	printf("Other thread: str (%p) = %s\n", str, str);

	return NULL;
}
