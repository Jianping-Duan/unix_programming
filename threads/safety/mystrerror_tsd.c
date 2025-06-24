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
#define MAX_THREADS	2

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t sekey;

struct thread_error {
	int te_thrno;
	int te_errno;
};

static char * mystrerror(int);
static void * thread_func(void *);
static void destructer(void *);
static void create_key(void);

int
main(void)
{
	pthread_t tids[MAX_THREADS];
	int r, i;
	char *str;

	struct thread_error tes[MAX_THREADS] = {
		{1, EINTR},
		{2, EPERM}
	};

	str = mystrerror(EINVAL);
	printf("Main thread has called mystrerror()\n");

	for (i = 0; i < MAX_THREADS; i++) {
		r = pthread_create(&tids[i], NULL, thread_func,
			(void *)&tes[i]);
		if (r != 0)
			errmsg_exit1("pthread_create failed, %d\n", r);
	}

	for (i = 0; i < MAX_THREADS; i++)
		if ((r = pthread_join(tids[i], NULL)) != 0)
			errmsg_exit1("pthread_join failed, %d\n", r);

	/*
	 * If mystrerror() is not thread-safe, then the output of this printf()
	 * be the same as that produced by the analogous printf() in
	 * thread_func()
	 */
	printf("Main thread strerror (%p) = %s\n", str, str);

	/*
	 * The pthread_key_delete() function deletes a thread-specific data key
	 * previously returned by pthread_key_create(). The thread-specific data
	 * values associated with key need not be NULL at the time that
	 * pthread_key_delete() is called. It is the responsibility of the
	 * application to free any application storage or perform any cleanup
	 * actions for data structures related to the deleted key or associated
	 * thread-specific data in any threads; this cleanup can be done either
	 * before or after pthread_key_delete() is called. Any attempt to use key
	 * following the call to pthread_key_delete() results in undefined behavior.
	 *
	 * The pthread_key_delete() function is callable from within destructor
	 * functions. Destructor functions are not invoked by
	 * pthread_key_delete(). Any destructor function that may have been
	 * associated with key will no longer be called upon thread exit.
	 */
	if ((r = pthread_key_delete(sekey)) != 0)
		errmsg_exit1("pthread_key_delete failed, %d\n", r);

	exit(EXIT_SUCCESS);
}

static char *
mystrerror(int eno)
{
	int r;
	char *buf;

	/*
	 * int
	 * pthread_once(pthread_once_t *once_control,
	 *	void (*init_routine)(void));
	 *
	 * The first call to pthread_once() by any thread in a process, with a
	 * given 'once_control', will call the 'init_routine()' with no
	 * arguments. Subsequent calls to pthread_once() with the same
	 * 'once_control' will not call the 'init_routine()'. On return from
	 * pthread_once(), it is guaranteed that 'init_routine()' has completed.
	 * The 'once_control' parameter is used to determine whether the
	 * associated initialization routine has been called
	 *
	 * The function pthread_once() is not a cancellation point. However, if
	 * 'init_routine()' is a cancellation point and is cancelled, the effect
	 * on 'once_control' is as if pthread_once() was never called.
	 */
	if ((r = pthread_once(&once, create_key)) != 0)
		errmsg_exit1("pthread_once failed, %d\n", r);

	/*
	 * The pthread_getspecific() function returns the value currently bound
	 * to the specified key on behalf of the calling thread.
	 *
	 * The effect of calling pthread_getspecific() with a key value not
	 * obtained from pthread_key_create() or after key has been deleted with
	 * pthread_key_delete() is undefined.
	 *
	 * The pthread_getspecific() function may be called from a
	 * thread-specific data destructor function. A call to
	 * pthread_getspecific() for the thread-specific data key being
	 * destroyed returns the value NULL, unless the value is changed
	 * (after the destructor starts) by a call to pthread_setspecific().
	 *
	 * The pthread_setspecific() function associates a thread-specific value
	 * with a key obtained via a previous call to pthread_key_create().
	 * Different threads can bind different values to the same key. These
	 * values are typically pointers to blocks of dynamically allocated
	 * memory that have been reserved for use by the calling thread
	 */
	if ((buf = (char *)pthread_getspecific(sekey)) == NULL) {
		buf = xmalloc(MAX_ERROR_LEN * sizeof(*buf));
		if ((r = pthread_setspecific(sekey, buf)) != 0)
			errmsg_exit1("pthread_setspecific failed, %d\n", r);
	}

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
	struct thread_error *te;

	te = (struct thread_error *)arg;
	printf("Thread %d calls mystrerror()\n", te->te_thrno);
	str = mystrerror(te->te_errno);
	printf("Thread %d: strerror (%p) = %s\n", te->te_thrno, str, str);

	return NULL;
}

static void
create_key(void)
{
	int r;

	/*
	 * The pthread_key_create() function creates a thread-specific data key
	 * visible to all threads in the process. Key values provided by
	 * pthread_key_create() are opaque objects used to locate
	 * thread-specific data. Although the same key value may be used by
	 * different threads, the values bound to the key by
	 * pthread_setspecific() are maintained on a per-thread basis and
	 * persist for the life of the calling thread.
	 *
	 * Upon key creation, the value NULL is associated with the new key in
	 * all active threads. Upon thread creation, the value NULL is
	 * associated with all defined keys in the new thread.
	 *
	 * An optional destructor function may be associated with each key
	 * value. At thread exit, if a key value has a non-NULL destructor
	 * pointer, and the thread has a non-NULL value associated with the key,
	 * the function pointed to is called with the current associated value
	 * as its sole argument. The order of destructor calls is unspecified if
	 * more than one destructor exists for a thread when it exits.
	 *
	 * If, after all the destructors have been called for all non-NULL
	 * values with associated destructors, there are still some non-NULL
	 * values with associated destructors, then the process is repeated. If,
	 * after at least [PTHREAD_DESTRUCTOR_ITERATIONS] iterations of
	 * destructor calls for outstanding non-NULL values, there are still
	 * some non-NULL values with associated destructors, the implementation
	 * stops calling destructors.
	 */
	if ((r = pthread_key_create(&sekey, destructer)) != 0)
		errmsg_exit1("pthread_key_create failed, %d\n", r);
}

static void
destructer(void *ptr)
{
	xfree(ptr);
}
