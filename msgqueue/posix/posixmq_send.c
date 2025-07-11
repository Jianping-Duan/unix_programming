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
	int flags, opt, ind;
	mqd_t mqd;
	unsigned int prio;

	extern int optind;
	extern char *optarg;

	flags = O_WRONLY;
	while ((opt = getopt(argc, argv, "n")) != -1)
		switch (opt) {
		case 'n':
			flags |= O_NONBLOCK;
			break;
		default:
			usage_info(argv[0]);
		}

	if (optind + 1 >= argc)
		usage_info(argv[0]);

	if ((mqd = mq_open(argv[optind], flags)) == (mqd_t)-1)
		errmsg_exit1("mq_open (%s) failed, %s\n", argv[optind],
			ERR_MSG);

	prio = (argc > optind + 2) ? getint(argv[optind + 2]) : 0;
	ind = optind + 1;
	/*
	 * The mq_send() system call adds the message pointed to by the argument
	 * msg_ptr to the message queue specified by mqdes. The msg_len argument
	 * specifies the length of the message, in bytes, pointed to by msg_ptr.
	 * The value of msg_len should be less than or equal to the mq_msgsize
	 * attribute of the message queue, or mq_send() will fail.
	 *
	 * If the specified message queue is not full, mq_send() will behave as
	 * if the message is inserted into the message queue at the position
	 * indicated by the msg_prio argument. A message with a larger numeric
	 * value of msg_prio will be inserted before messages with lower values
	 * of msg_prio. A message will be inserted after other messages in the
	 * queue, if any, with equal msg_prio. The value of msg_prio should be
	 * less than {MQ_PRIO_MAX}.
	 *
	 * If the specified message queue is full and O_NONBLOCK is not set in
	 * the message queue description associated with mqdes, mq_send() will
	 * block until space becomes available to enqueue the message, or until
	 * mq_send() is interrupted by a signal. If more than one thread is
	 * waiting to send when space becomes available in the message queue and
	 * the Priority Scheduling option is supported, then the thread of the
	 * highest priority that has been waiting the longest will be unblocked
	 * to send its message. Otherwise, it is unspecified which waiting
	 * thread is unblocked. If the specified message queue is full and
	 * O_NONBLOCK is set in the message queue description associated with
	 * mqdes, the message will not be queued and mq_send() will return an
	 * error.
	 */
	if (mq_send(mqd, argv[ind], strlen(argv[ind]), prio) == -1)
		errmsg_exit1("mq_send failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname)
{
	fprintf(stderr, "Usage: %s [-n] msg-name msg [prio]\n", pname);
	fprintf(stderr, "\t-n\tUse O_NONBLOCK flag\n");

	exit(EXIT_FAILURE);
}
