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
#ifndef _FIFOSEQNUM_H_
#define _FIFOSEQNUM_H_

/* Well-known name for server's FIFO */
#define SERVER_FIFO		"/tmp/seqnum_sv"

/* Template for building client FIFO name */
#define CLIENT_FIFO_TEMPLATE	"/tmp/seqnum_cl.%d"

/* Space required for client FIFO pathname */
/* (+20 as a generous allowance for the PID) */
#define CLIENT_FIFO_LEN		(sizeof(CLIENT_FIFO_TEMPLATE) + 20)

/* Request (client --> server) */
struct fifo_request {
	pid_t	fr_pid;		/* PID of client */
	int	fr_seqlen;	/* Length of desired sequence */
};

/* Response (server --> client) */
struct fifo_response {
	int	fr_seqnum;	/* Start of sequence */
};

static const int fifo_req_size = sizeof(struct fifo_request);
static const int fifo_resp_size = sizeof(struct fifo_response);

#endif
