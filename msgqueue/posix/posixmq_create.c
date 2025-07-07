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
#include <sys/stat.h>
#include <fcntl.h>

static void usage_info(const char *);

int
main(int argc, char *argv[])
{
	int flags, opt;
	mode_t perms;
	mqd_t mqd;
	struct mq_attr attr, *attrp = NULL;

	const char *optstr = "cm:s:xn";

	extern int optind;
	extern char *optarg;

	/*
	 * If 'attrp' is NULL, mq_open() uses default attributes. If an option
	 * specifying a message queue attribute is supplied on the command line,
	 * we save the attribute in 'attr' and set 'attrp' pointing to 'attr'.
	 * We assign some (arbitrary) default values to the fields of 'attr' in
	 * case the user specifies the value for one of the queue attributes,
	 * but not the other. 
	 */
	attr.mq_maxmsg = 16;
	attr.mq_msgsize = 2048;
	flags = O_RDWR;

	while ((opt = getopt(argc, argv, optstr)) != -1) {
		switch (opt) {
		case 'c':
			flags |= O_CREAT;
			break;
		case 'm':
			if (sscanf(optarg, "%ld", &attr.mq_maxmsg) != 1)
				errmsg_exit1("Illeage number, -m %s\n", optarg);
			attrp = &attr;
			break;
		case 's':
			attr.mq_msgsize = atoi(optarg);
			attrp = &attr;
			break;
		case 'x':
			flags |= O_EXCL;
			break;
		case 'n':
			flags |= O_NONBLOCK;
			break;
		default:
			usage_info(argv[0]);
		}
	}

	if (optind >= argc)
		usage_info(argv[0]);

	perms = (argc <= optind + 1) ? (S_IRUSR | S_IWUSR) :
		getlong(argv[optind + 1], GN_BASE_8);

	/*
	 * The mq_open() system call establishes the connection between a
	 * process and a message queue with a message queue descriptor. It
	 * creates an open message queue description that refers to the message
	 * queue, and a message queue descriptor that refers to that open
	 * message queue description. The message queue descriptor is used by
	 * other functions to refer to that message queue. The name argument
	 * points to a string naming a message queue. The name argument should
	 * conform to the construction rules for a pathname. The name should
	 * begin with a slash character. Processes calling mq_open() with the
	 * same value of name refers to the same message queue object, as long
	 * as that name has not been removed. If the name argument is not the
	 * name of an existing message queue and creation is not requested,
	 * mq_open() will fail and return an error.
	 *
	 * The oflag argument requests the desired receive and/or send access to
	 * the message queue. The requested access permission to receive
	 * messages or send messages would be granted if the calling process
	 * would be granted read or write access, respectively, to an
	 * equivalently protected file.
	 *
	 * The value of oflag is the bitwise-inclusive OR of values from the
	 * following list. Applications should specify exactly one of the first
	 * three values (access modes) below in the value of oflag:
	 *
	 * O_RDONLY
	 *	Open the message queue for receiving messages. The process can
	 *	use the returned message queue descriptor with mq_receive(),
	 *	but not mq_send(). A message queue may be open multiple times in
	 *	the same or different processes for receiving messages.
	 *
	 * O_WRONLY
	 *	Open the queue for sending messages. The process can use the
	 *	returned message queue descriptor with mq_send() but not
	 *	mq_receive(). A message queue may be open multiple times in
	 *	the same or different processes for sending messages.
	 *
	 * O_RDWR
	 *	Open the queue for both receiving and sending messages. The
	 *	process can use any of the functions allowed for O_RDONLY and
	 *	O_WRONLY. A message queue may be open multiple times in the
	 *	same or different processes for sending messages.
	 *
	 * Any combination of the remaining flags may be specified in the value
	 * of oflag:
	 *
	 * O_CREAT
	 *	Create a message queue. It requires two additional arguments:
	 *	mode, which is of type mode_t, and attr, which is a pointer
	 *	to an mq_attr structure. If the pathname name has already been
	 *	used to create a message queue that still exists, then this flag
	 *	has no effect, except as noted under O_EXCL. Otherwise, a
	 *	message queue will be created without any messages in it. The
	 *	user ID of the message queue will be set to the effective user
	 *	ID of the process, and the group ID of the message queue will be
	 *	set to the effective group ID of the process. The permission
	 *	bits of the message queue will be set to the value of the mode
	 *	argument, except those set in the file mode creation mask of the
	 *	process. When bits in mode other than the file permission bits
	 *	are specified, the effect is unspecified. If attr is NULL, the
	 *	message queue is created with implementation-defined default
	 *	message queue attributes. If attr is non-NULL and the calling
	 *	process has the appropriate privilege on name, the message queue
	 *	mq_maxmsg and mq_msgsize attributes will be set to the values of
	 *	the corresponding members in the mq_attr structure referred to
	 *	by attr. If attr is non-NULL, but the calling process does not
	 *	have the appropriate privilege on name, the mq_open() function
	 *	will fail and return an error without creating the message
	 *	queue.
	 *
	 * O_EXCL
	 *	If O_EXCL and O_CREAT are set, mq_open() will fail if the
	 *	message queue name exists.
	 *
	 * O_NONBLOCK
	 *	Determines whether an mq_send() or mq_receive() waits for
	 *	resources or messages that are not currently available, or fails
	 *	with errno set to EAGAIN; see mq_send(2) and mq_receive(2) for
	 *	details
	 *
	 * Upon successful completion, the function returns a message queue
	 * descriptor; otherwise, the function returns (mqd_t)-1 and sets the
	 * global variable errno to indicate the error.
	 */
	mqd = mq_open(argv[optind], flags, perms, attrp);
	if (mqd == (mqd_t)-1)
		errmsg_exit1("mq_open failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname)
{
	fprintf(stderr, "Usage %s [-cx] [-m maxmsgs] [-s msgsize] mq-name "
		"[octal-perms]\n", pname);
	fprintf(stderr, "\t-c\tCreate queue (O_CREAT)\n");
	fprintf(stderr, "\t-x\tCreate exclusively (O_EXCL)\n");
	fprintf(stderr, "\t-m\tSet maximum # of messages\n");
	fprintf(stderr, "\t-s\tSet maxinum message size\n");
	fprintf(stderr, "\t-n\tUse O_NONBLOCK flag\n");
	
	exit(EXIT_FAILURE);
}
