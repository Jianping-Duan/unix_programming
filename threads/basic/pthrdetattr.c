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

struct pthdata {
	int pth_key;
	char pth_val[32];
};

static void * thread_func(void *);

int
main(void)
{
	pthread_t pthr;
	pthread_attr_t ptattr;
	int rv;
	struct pthdata dat = {12089, "This is a pthread attribute demo"};

	/*
	 * More details see pthread_attr_*(3)
	 */

	if (pthread_attr_init(&ptattr) != 0)
		errmsg_exit1("pthread_attr_init failed, %s\n", ERR_MSG);

	rv = pthread_attr_setdetachstate(&ptattr, PTHREAD_CREATE_DETACHED);
	if (rv != 0)
		errmsg_exit1("pthread_attr_setdetachstate failed, %s\n",
			ERR_MSG);

	rv = pthread_create(&pthr, &ptattr, thread_func, (void *)&dat);
	if (rv != 0)
		errmsg_exit1("pthread_create failed, %s\n", ERR_MSG);

	if (pthread_attr_destroy(&ptattr) != 0)
		errmsg_exit1("pthread_attr_destroy failed, %s\n", ERR_MSG);

	if (pthread_join(pthr, NULL) != 0)
		errmsg_exit1("pthread_join failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}

static void *
thread_func(void *arg)
{
	struct pthdata *dat = (struct pthdata *)arg;

	printf("pth_key = %d\n", dat->pth_key);
	printf("pth_val = %s\n", dat->pth_val);

	return (void *)0;
}
