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
#ifndef _SYSVMQ_FILE_H_
#define _SYSVMQ_FILE_H_

#include <stddef.h>	/* offsetof() */

#define SVMQ_SERVER_KEY		0x1aaa0aaa	/* Key for server's MQ */

/* Requests (client --> server) */
struct svmq_request {
	long	mtype;			/* Unused */
	int	clientid;		/* ID of client's message queue */
#ifndef PATH_MAX
#define PATH_MAX	255
#endif
	char	pathname[PATH_MAX];	/* File to be returned */
};

/*
 * SVMQ_REQ_SIZE computes size of 'mtext' part of 'svmq_request' structure.
 * We use offsetof() to handle the possibility that there are padding bytes
 * between the 'clientId' and 'pathname' fields.
 */
#define SVMQ_REQ_SIZE	(offsetof(struct svmq_request, pathname) - \
	offsetof(struct svmq_request, clientid) + PATH_MAX)

/* Responses (server --> client) */
struct svmq_response {
#define SVMQ_RESP_FAILURE	1	/* File couldn't be opened */
#define SVMQ_RESP_DATA		2	/* Message contains file data */
#define SVMQ_RESP_END		3	/* File data complete */
	long	mtype;			/*  One of SVMQ_RESP_* values above */
#define SVMQ_RESP_SIZE		2048
	char	data[SVMQ_RESP_SIZE];	/* File content / response message */

};

static inline void
removemq(int msqid)
{
	if (msgctl(msqid, IPC_RMID, NULL) == -1)
		 errmsg_exit1("msgctl - IPC_RMID failed, %s\n", ERR_MSG);
}

#endif	/* !_SYSVMQ_FILE_H_ */
