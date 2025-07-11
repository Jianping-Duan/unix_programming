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
#include <signal.h>
#include <fcntl.h>

#define NOTIFY_SIG	SIGUSR1

static volatile sig_atomic_t gotsig = 1;

static void sig_handler(int);

int
main(int argc, char *argv[])
{
	mqd_t mqd;
	struct mq_attr attr;
	struct sigaction sa;
	struct sigevent sev;
	char *buf;
	ssize_t nrd;
	long msgsz;
	unsigned int prio;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s mq-name\n", argv[0]);

	/*
	 * Open the (existing) queue in nonblocking mode so that we can drain
	 * messages from it without blocking once the queue has been emptied 
	 */

	if ((mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK)) == (mqd_t)-1)
		errmsg_exit1("mq_open (%s) failed, %s\n", argv[1], ERR_MSG);

	/* Establish handler for notification signal */

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	if (sigaction(NOTIFY_SIG, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	/*
	 * Determine mq_msgsize for message queue, and allocate an input buffer
	 * of that size
	 */

	if (mq_getattr(mqd, &attr) == -1)
		errmsg_exit1("mq_getattr failed, %s\n", ERR_MSG);
	msgsz = attr.mq_msgsize;
	buf = xmalloc(msgsz);


	/*
	 * If the argument notification is not NULL, this system call will
	 * register the calling process to be notified of message arrival at an
	 * empty message queue associated with the specified message queue
	 * descriptor, mqdes. The notification specified by the notification
	 * argument will be sent to the process when the message queue
	 * transitions from empty to non-empty. At any time, only one process
	 * may be registered for notification by a message queue. If the calling
	 * process or any other process has already registered for notification
	 * of message arrival at the specified message queue, subsequent
	 * attempts to register for that message queue will fail.
	 *
	 * The notification argument points to a sigevent structure that defines
	 * how the calling process will be notified. If
	 * notification->sigev_notify is SIGEV_NONE, then no signal will be
	 * posted, but the error status and the return status for the operation
	 * will be set appropriately. For SIGEV_SIGNO and SIGEV_THREAD_ID
	 * notifications, the signal specified in notification->sigev_signo will
	 * be sent to the calling process (SIGEV_SIGNO) or to the thread whose
	 * LWP ID is notification->sigev_notify_thread_id (SIGEV_THREAD_ID). The
	 * information for the queued signal will include:
	 *
	 * Member          Value
	 * si_code         SI_MESGQ
	 * si_value        the value stored in notification->sigev_value
	 * si_mqd          mqdes
	 *
	 * If notification is NULL and the process is currently registered for
	 * notification by the specified message queue, the existing
	 * registration will be removed.
	 *
	 * When the notification is sent to the registered process, its
	 * registration is removed. The message queue then is available for
	 * registration.
	 *
	 * If a process has registered for notification of message arrival at a
	 * message queue and some thread is blocked in mq_receive() waiting to
	 * receive a message when a message arrives at the queue, the arriving
	 * message will satisfy the appropriate mq_receive(). The resulting
	 * behavior is as if the message queue remains empty, and no
	 * notification will be sent.
	 * 
	 */

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = NOTIFY_SIG;

	while (1) {
		if (!gotsig)
			goto nogotsig;

		gotsig = 0;

		/* Reregister for message notification */
		if (mq_notify(mqd, &sev) == -1)
			errmsg_exit1("mq_notify (2) failed, %s\n", ERR_MSG);
		
		/* Drain all messages from the queue */

		while ((nrd = mq_receive(mqd, buf, msgsz, &prio)) >= 0)
			printf("Read %ld bytes; priority = %u\n", nrd, prio);

		/*
		 * O_NONBLOCK flag is set in the message queue
		 * description associated with mqdes, and the specified
		 * message queue is empty.
		 */
		if (errno != EAGAIN) {	/* Unexpected error */
			fprintf(stderr, "mq_receive failed, %s\n", ERR_MSG);
			break;
		}

nogotsig:
	
		sleep(5);	/* do some work */
	}

	xfree(buf);

	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	/* Just interrupt sigsuspend() */
	assert(sig == NOTIFY_SIG);
	gotsig = 1;
}
