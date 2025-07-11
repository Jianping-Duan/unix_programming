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
/*
 * NOTES
 *
 * FreeBSD implements message queue based on file descriptor. The descriptor is
 * inherited by child after fork(2). The descriptor is closed in a new image
 * after exec(3). The select(2) and kevent(2) system calls are supported for
 * message queue descriptor.
 *
 * Please see the mqueuefs(5) man page for instructions on loading the module
 * or compiling the service into the kernel.
 */
#include "unibsd.h"
#include <mqueue.h>
#include <fcntl.h>

static void notify_startup(mqd_t *);
static void drain_queue(mqd_t);
static void thread_notify(union sigval);

int
main(int argc, char *argv[])
{
	mqd_t mqd;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage %s mq-name\n", argv[0]);

	if ((mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK)) == (mqd_t)-1)
		errmsg_exit1("mq_open (%s) failed, %s\n", argv[1], ERR_MSG);

	notify_startup(&mqd);
	drain_queue(mqd);

	pause();	/* Wait for notifications via thread function */

	exit(EXIT_SUCCESS);
}

/* Reregister for message notification */
static void
notify_startup(mqd_t *mqd)
{
	struct sigevent sev;

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = thread_notify;
	sev.sigev_notify_attributes = NULL;	/* pointer to pthread_attr_t */
	sev.sigev_value.sigval_ptr = mqd;	/* Argument to threadFunc() */

	if (mq_notify(*mqd, &sev) == -1)
		errmsg_exit1("mq_notify failed, %s\n", ERR_MSG);
}

/* Drain all messages from the queue referred to by 'mqd' */
static void
drain_queue(mqd_t mqd)
{
	struct mq_attr attr;
	ssize_t nrd, msgsz;
	char *buf;
	unsigned int prio;

	if (mq_getattr(mqd, &attr) == -1)
		errmsg_exit1("mq_getattr failed, %s\n", ERR_MSG);
	msgsz = attr.mq_msgsize;
	buf = xmalloc(msgsz);
	
	while ((nrd = mq_receive(mqd, buf, msgsz, &prio)) >= 0)
		printf("Read %ld bytes; priority = %u\n", nrd, prio);

	if (errno != EAGAIN)
		errmsg_exit1("mq_receive failed, %s\n", ERR_MSG);

	xfree(buf);
}

/* Thread notification function */
static void
thread_notify(union sigval sv)
{
	mqd_t *mqd = (mqd_t *)sv.sigval_ptr;

	notify_startup(mqd);
	drain_queue(*mqd);
}
