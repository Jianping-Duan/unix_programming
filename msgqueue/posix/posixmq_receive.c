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

static void usage_info(const char *);

int
main(int argc, char *argv[])
{
	int flags, opt;
	mqd_t mqd;
	unsigned int prio;
	char *buf;
	struct mq_attr attr;
	ssize_t nrd;

	extern int optind;
	extern char *optarg;

	flags = O_RDONLY;
	while ((opt = getopt(argc, argv, "n")) != -1)
		switch (opt) {
		case 'n':
			flags |= O_NONBLOCK;
			break;
		default:
			usage_info(argv[0]);
		}

	if (optind >= argc)
		usage_info(argv[0]);

	if ((mqd = mq_open(argv[optind], flags)) == (mqd_t)-1)
		errmsg_exit1("mq_open (%s) failed, %s\n", argv[optind],
			ERR_MSG);

	/*
	 * We need to know the 'mq_msgsize' attribute of the queue in
	 * order to determine the size of the buffer for mq_receive()
	 */
	if (mq_getattr(mqd, &attr) == -1)
		errmsg_exit1("mq_getattr failed, %s\n", ERR_MSG);

	buf = xmalloc(attr.mq_msgsize);
	/*
	 * The mq_receive() system call receives oldest of the highest priority
	 * message(s) from the message queue specified by mqdes. If the size of
	 * the buffer in bytes, specified by the msg_len argument, is less than
	 * the mq_msgsize attribute of the message queue, the system call will
	 * fail and return an error. Otherwise, the selected message will be
	 * removed from the queue and copied to the buffer pointed to by the
	 * msg_ptr argument.
	 *
	 * If the argument msg_prio is not NULL, the priority of the selected
	 * message will be stored in the location referenced by msg_prio. If
	 * the specified message queue is empty and O_NONBLOCK is not set in
	 * the message queue description associated with mqdes, mq_receive()
	 * will block until a message is enqueued on the message queue or until
	 * mq_receive() is interrupted by a signal. If more than one thread is
	 * waiting to receive a message when a message arrives at an empty queue
	 * and the Priority Scheduling option is supported, then the thread of
	 * highest priority that has been waiting the longest will be selected
	 * to receive the message. Otherwise, it is unspecified which waiting
	 * thread receives the message. If the specified message queue is empty
	 * and O_NONBLOCK is set in the message queue description associated
	 * with mqdes, no message will be removed from the queue, and
	 * mq_receive() will return an error.
	 */
	if ((nrd = mq_receive(mqd, buf, attr.mq_msgsize, &prio)) == -1)
		errmsg_exit1("mq_receive failed, %s\n", ERR_MSG);
	printf("Read %ld bytes; priority = %u\n", nrd, prio);

	if (write(STDOUT_FILENO, buf, nrd) == -1)
		errmsg_exit1("write failed, %s\n", ERR_MSG);
	assert(write(STDOUT_FILENO, "\n", 1) == 1 == 1);

	xfree(buf);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname)
{
	fprintf(stderr, "Usage: %s [-n] msg-name\n", pname);
	fprintf(stderr, "\t-n\tUse O_NONBLOCK flag\n");

	exit(EXIT_FAILURE);
}
