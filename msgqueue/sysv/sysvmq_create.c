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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

static void usage_info(const char *, const char *);

int
main(int argc, char *argv[])
{
	const char *optstr = "cf:k:px";
	int opt, flags = 0, kcnt = 0, mqid;
	mode_t perms;
	key_t key;
	long ukey;

	extern char *optarg;
	extern int optind;

	while ((opt = getopt(argc, argv, optstr)) != -1) {
		switch (opt) {
		case 'c':
			flags |= IPC_CREAT;
			break;
		case 'f':
			if ((key = ftok(optarg, 1)) == -1)
				errmsg_exit1("ftok failed\n");
			kcnt++;
			break;
		case 'k':
			if (sscanf(optarg, "%ld", &ukey) != 1)
				errmsg_exit1("-k option requires a numeric "
					"argument, %s\n", optarg);
			key = ukey;
			kcnt++;
			break;
		case 'p':
			key = IPC_PRIVATE;
			kcnt++;
			break;
		case 'x':
			flags |= IPC_EXCL;
			break;
		default:
			usage_info(argv[0], "Bad option");
		}
	}

	if (kcnt != 1)
		usage_info(argv[0], "Exactly one of the options -f, -k, or -p "
			"must be supplied\n");

	perms = (optind == argc) ? (S_IRUSR | S_IWUSR) :
		getlong(argv[optind], GN_BASE_8);

	/*
	 * The msgget() function returns the message queue identifier associated
	 * with key. A message queue identifier is a unique integer greater than
	 * zero.
	 *
	 * A message queue is created if either key is equal to IPC_PRIVATE, or
	 * key does not have a message queue identifier associated with it, and
	 * the IPC_CREAT bit is set in msgflg.
	 *
	 * If a new message queue is created, the data structure associated with
	 * it (the msqid_ds structure, see msgctl(2)) is initialized as follows:
	 *
	 *	msg_perm.cuid and msg_perm.uid are set to the effective uid of
	 *	the calling process.
	 *
	 *	msg_perm.gid and msg_perm.cgid are set to the effective gid of
	 *	the calling process.
	 *
	 *	msg_perm.mode is set to the lower 9 bits of msgflg which are set
	 *	by ORing these constants:
	 *
	 *		0400  Read access for user.
	 *		0200  Write access for user.
	 *		0040  Read access for group.
	 *		0020  Write access for group.
	 *		0004  Read access for other.
	 *		0002  Write access for other.
	 *
	 *	msg_cbytes, msg_qnum, msg_lspid, msg_lrpid, msg_rtime, and
	 *	msg_stime are set to 0.
	 *
	 *	msg_qbytes is set to the system wide maximum value for the
	 *	number of bytes in a queue (MSGMNB)
	 *
	 *	msg_ctime is set to the current time.
	 *
	 * Upon successful completion a positive message queue identifier is
	 * returned. Otherwise, -1 is returned and the global variable errno is
	 * set to indicate the error.
	 */
	if ((mqid = msgget(key, flags | perms)) == -1)
		errmsg_exit1("msgget failed, %s\n", ERR_MSG);
	printf("%d\n", mqid);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname, const char *msg)
{
	if (msg != NULL)
		fprintf(stderr, "%s\n", msg);

	fprintf(stderr, "Usage: %s [-cx] {-f pathname | -k key | -p} "
		"[octal-perms]\n", pname);
	fprintf(stderr, "\t-c\t\tUse IPC_CREAT flag\n");
	fprintf(stderr, "\t-x\t\tUse IPC_EXCl flag\n");
	fprintf(stderr, "\t-f pathname Generate key using ftok\n");
	fprintf(stderr, "\t-k key\tUse 'key' as key\n");
	fprintf(stderr, "\t-p\t\tUse IPC_PRIVATE key\n");
	exit(EXIT_FAILURE);
}
