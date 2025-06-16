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
#include <signal.h>

#define CMDLEN	128

int
main(void)
{
	pid_t cpid;
	char cmd[CMDLEN];

	setbuf(stdout, NULL);	/* Disable buffering of stdout */

	printf("Parent: PID = %d\n", getpid());
	printf("\n");

	if ((cpid = fork()) == -1)
		errmsg_exit1("fork failed, %s\n", ERR_MSG);
	else if (cpid == 0) {
		/* Child: immediately exits to become zombie */
		printf("Child (PID = %d) exiting\n", getpid());
		_exit(EXIT_SUCCESS);
	} else {
		sleep(3);	/* Give child a chance to start and exit */
		snprintf(cmd, CMDLEN, "ps | grep -e %d -e %d", cpid, getpid());
		system(cmd);
		printf("\n");

		/* Now send the "sure kill" signal to the zombie */
		if (kill(cpid, SIGKILL) == -1)
			errmsg_exit1("kill - SIGKILL failed, %s\n", ERR_MSG);
		sleep(3);	/* Give child a chance to react to signal */
		printf("After sending a SIGKILL to zombie (PID = %d)\n", cpid);
		system(cmd);

		exit(EXIT_SUCCESS);
	}
}
