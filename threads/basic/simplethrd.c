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

static void * mythread(void *);

int
main(void)
{
	pthread_t t1;
	int r;
	char str[] = "Simple Thread";
	void *valptr;

	/*
	 * The pthread_create() function is used to create a new thread, with
	 * attributes specified by attr, within a process.  If attr is NULL, the
	 * default attributes are used.  If the attributes specified by attr are
	 * modified later, the thread's attributes are not affected. Upon 
	 * successful completion pthread_create() will store the ID of the
	 * created thread in the location specified by thread.
	 *
	 * The thread is created executing start_routine with arg as its sole
	 * argument. If the start_routine returns, the effect is as if there
	 * was an implicit call to pthread_exit() using the return value of
	 * start_routine as the exit status. Note that the thread in which
	 * main() was originally invoked differs from this. When it returns
	 * from main(), the effect is as if there was an implicit call to exit()
	 * using the return value of main() as the exit status.
	 */
	r = pthread_create(&t1, NULL, mythread, (void *)str);
	if (r != 0)
		errmsg_exit1("pthread_create failed, %s\n", ERR_MSG);

	printf("Message from main()\n");
	/*
	 * The pthread_join() function suspends execution of the calling thread
	 * until the target thread terminates unless the target thread has
	 * already terminated.
	 *
	 * On return from a successful pthread_join() call with a non-NULL
	 * value_ptr argument, the value passed to pthread_exit() by the
	 * terminating thread is stored in the location referenced by value_ptr.
	 * When a pthread_join() returns successfully, the target thread has
	 * been terminated. The results of multiple simultaneous calls to
	 * pthread_join() specifying the same target thread are undefined. If
	 * the thread calling pthread_join() is cancelled, then the target
	 * thread is not detached.
	 */
	if (pthread_join(t1, &valptr) != 0)
		errmsg_exit1("pthread_join failed, %s\n", ERR_MSG);
	printf("Thread returned %d\n", *((int *)valptr));
	xfree(valptr);

	exit(EXIT_SUCCESS);
}

static void *
mythread(void *arg)
{
	char *str = (char *)arg;
	int *n;

	printf("%s\n", str);
	n = xmalloc(sizeof(*n));
	*n = (int)strlen(str);

	return (void *)n;
}
