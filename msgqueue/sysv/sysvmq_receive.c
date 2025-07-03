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
	int flags = 0, typ = 0, mqid, opt;
	int maxsz, msgsz;
	struct mqbuf msg;

	extern char *optarg;
	extern int optind;

	while ((opt = getopt(argc, argv, "ent:x")) != -1)
		switch (opt) {
		case 'e':
			flags |= MSG_NOERROR;
			break;
		case 't':
			typ = getint(optarg);
			break;
		case 'n':
			flags |= IPC_NOWAIT;
			break;
		default:
			usage_info(argv[0], NULL);
		}

	if (argc < optind + 1 || argc > optind + 2)
		usage_info(argv[0], "Wrong number of arguments");

	mqid = getint(argv[optind]);
	maxsz = (argc > optind + 1) ? getint(argv[optind + 1]) : MAX_TEXT_SIZE;

	/*
	 * The msgrcv() function receives a message from the message queue
	 * specified in msqid, and places it into the structure pointed to by
	 * msgp. This structure should consist of the following members:
	 *
	 *	long mtype;
	 *	char mtext[1];
	 *
	 * mtype is an integer greater than 0 that can be used for selecting
	 * messages, mtext is an array of bytes, with a size up to that of the
	 * system limit (MSGMAX).
	 *
	 * The value of msgtyp has one of the following meanings:
	 *
	 *	The msgtyp argument is greater than 0. The first message of type
	 *	msgtyp will be received.
	 *
	 *	The msgtyp argument is equal to 0. The first message on the
	 *	queue will be received.
	 *
	 *	The msgtyp argument is less than 0. The first message of the
	 *	lowest message type that is less than or equal to the absolute
	 *	value of msgtyp will be received.
	 *
	 * The msgsz argument specifies the maximum length of the requested
	 * message. If the received message has a length greater than msgsz it
	 * will be silently truncated if the MSG_NOERROR flag is set in msgflg,
	 * otherwise an error will be returned.
	 *
	 * If no matching message is present on the message queue specified by
	 * msqid, the behavior of msgrcv() depends on whether the IPC_NOWAIT
	 * flag is set in msgflg or not. If IPC_NOWAIT is set, msgrcv() will
	 * immediately return a value of -1, and set errno to ENOMSG. If
	 * IPC_NOWAIT is not set, the calling process will be blocked until:
	 *
	 *	A message of the requested type becomes available on the message
	 *	queue.
	 *
	 *	The message queue is removed, in which case -1 will be returned,
	 *	and errno set to EINVAL.
	 *
	 *	A signal is received and caught. -1 is returned, and errno set
	 *	to EINTR.
	 *
	 * If a message is successfully received, the data structure associated
	 * with msqid is updated as follows:
	 *
	 *	msg_cbytes is decremented by the size of the message.
	 *
	 *	msg_lrpid is set to the pid of the caller.
	 *
	 *	msg_lrtime is set to the current time.
	 *
	 *	msg_qnum is decremented by 1.
	 */
	if ((msgsz = msgrcv(mqid, &msg, maxsz, typ, flags)) == -1)
		errmsg_exit1("msgrcv failed, %s\n", ERR_MSG);
	printf("Received: type=%ld; size=%d\n", msg.mtype, msgsz);
	if (msgsz > 0)
		printf("\tbody=%s\n", msg.mtext);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname, const char *msg)
{
	if (msg != NULL)
		fprintf(stderr, "%s\n", msg);

	fprintf(stderr, "Usage %s [opts] mqid [max-bytes]\n", pname);
	fprintf(stderr, "\t-e\tUse MSG_NOERROR flag\n");
	fprintf(stderr, "\t-t\tSelect message of given type\n");
	fprintf(stderr, "\t-n\tUse IPC_NOWAIT flag\n");

	exit(EXIT_FAILURE);
}
