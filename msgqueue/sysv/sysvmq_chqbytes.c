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
#include <sys/msg.h>

int
main(int argc, char *argv[])
{
	struct msqid_ds mqds;
	int mqid;

	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s msqid max-bytes\n", argv[0]);

	/*
	 * IPC_STAT:
	 *	Gather information about the message queue and place it in the
	 *	structure pointed to by buf.
	 *
	 * IPC_SET:
	 *	Set the value of the msg_perm.uid, msg_perm.gid, msg_perm.mode
	 *	and msg_qbytes fields in the structure associated with msqid.
	 *	The values are taken from the corresponding fields in the
	 *	structure pointed to by buf. This operation can only be
	 *	executed by the super-user, or a process that has an effective
	 *	user id equal to either msg_perm.cuid or msg_perm.uid in the
	 *	data structure associated with the message queue. The value
	 *	of msg_qbytes can only be increased by the super-user. Values
	 *	or msg_qbytes that exceed the system limit (MSGMNB from
	 *	<sys/msg.h>) are silently truncated to that limit.
	 *
	 * More details see msgctl(2)
	 */

	mqid = getint(argv[1]);
	if (msgctl(mqid, IPC_STAT, &mqds) == -1)
		errmsg_exit1("msgctl - IPC_STAT failed, %s\n", ERR_MSG);

	printf("Original msg_qbytes = %ld\n", mqds.msg_qbytes);

	mqds.msg_qbytes = getint(argv[1]);
	if (msgctl(mqid, IPC_SET, &mqds) == -1)
		errmsg_exit1("msgctl - IPC_SET failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
