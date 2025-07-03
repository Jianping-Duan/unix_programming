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
#include <getopt.h>
#include <sys/msg.h>

struct mqbuf {
	long mtype;			/* Message type */
#define MAX_TEXT_SIZE	1024
	char mtext[MAX_TEXT_SIZE];	/* Message body */
};

static void usage_info(const char *, const char *);

int
main(int argc, char *argv[])
{
	int opt, flags = 0, mqid, msgsz;
	struct mqbuf msg;

	extern int optind;
	extern char *optarg;

	while ((opt = getopt(argc, argv, "n")) != -1)
		if (opt == 'n')
			flags |= IPC_NOWAIT;
		else
			usage_info(argv[0], NULL);
	
	if (argc < optind + 2 || argc > optind + 3)
		usage_info(argv[0], "Wrong number of arguments");

	mqid = getint(argv[optind]);
	msg.mtype = getint(argv[optind + 1]);

	if (argc <= optind + 2) {
		msgsz = 0;
		goto notext;
	}

	msgsz = strlen(argv[optind + 2]) + 1;	/* include '\0' */
	if (msgsz > MAX_TEXT_SIZE)
		errmsg_exit1("msg-text too long (max: %d bytes)\n",
			MAX_TEXT_SIZE);
	memcpy(msg.mtext, argv[optind + 2], msgsz);

notext:

	/*
	 * The msgsnd() function sends a message to the message queue specified
	 * in msqid. The msgp argument points to a structure containing the
	 * message. This structure should consist of the following members:
	 *	
	 *	long mtype;
	 *	char mtext[1];
	 *
	 * mtype is an integer greater than 0 that can be used for selecting
	 * messages (see msgrcv(2)), mtext is an array of msgsz bytes. The
	 * argument msgsz can range from 0 to a system-imposed maximum, MSGMAX.
	 *
	 * If the number of bytes already on the message queue plus msgsz is
	 * bigger than the maximum number of bytes on the message queue
	 * (msg_qbytes, see msgctl(2)), or the number of messages on all queues
	 * system-wide is already equal to the system limit, msgflg determines
	 * the action of msgsnd(). If msgflg has IPC_NOWAIT mask set in it, the
	 * call will return immediately. If msgflg does not have IPC_NOWAIT set
	 * in it, the call will block until:
	 *
	 *	 The condition which caused the call to block does no longer
	 *	 exist.
	 *	 The message queue is removed, in which case -1 will be
	 *	 returned, and errno is set to EINVAL.
	 *	 The caller catches a signal. The call returns with errno set to
	 *	 EINTR.
	 *
	 * After a successful call, the data structure associated with the
	 * message queue is updated in the following way:
	 *
	 *	msg_cbytes is incremented by the size of the message.
	 *	msg_qnum is incremented by 1.
	 *	msg_lspid is set to the pid of the calling process.
	 *	msg_stime is set to the current time.
	 */
	if (msgsnd(mqid, &msg, msgsz, flags) == -1)
		errmsg_exit1("msgsnd failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname, const char *msg)
{
	if (msg != NULL)
		fprintf(stderr, "%s\n", msg);

	fprintf(stderr, "Usage: %s [-n] mqid msg-typ [msg-text]\n", pname);
	fprintf(stderr, "\t-n\tUse IPC_NOWAIT flag\n");
	exit(EXIT_FAILURE);
}
