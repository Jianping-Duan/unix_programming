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
#ifndef _SOCKID_SEQNUM_H_
#define _SOCKID_SEQNUM_H_

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define DFT_PORT_NUM	"20100"	/* Default port number for server */
#define INT_LEN		30	/* Size of string able to hold largest integer
				(including terminating '\n') */

static ssize_t
readline(int fd, void *buf, ssize_t sz)
{
	ssize_t nrd, totrd = 0;
	char *ptr = (char *)buf, ch;

	if (sz <= 0 || buf == NULL) {
		errno = EINVAL;
		return -1;
	}

	while (1) {
		if ((nrd = read(fd, &ch, 1)) == -1) {
			if (errno == EINTR)	/* Interrupted, restart read */
				continue;
			else
				return -1;
		} else if (nrd == 0) {		/* EOF */
			if (totrd == 0)		/* No bytes read */
				return 0;
			else
				break;
		} else {
			if (totrd < sz -1) {	/* Discard > (sz - 1) bytes */
				totrd++;
				*ptr++ = ch;
			}

			if (ch == '\n')
				break;
		}
	}

	*ptr = '\0';

	return totrd;
}

#endif	/* !_SOCKID_SEQNUM_H_ */
