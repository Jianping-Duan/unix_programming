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

static void tstp_handler(int);

int
main(void)
{
	struct sigaction sa;

	/* Only establish handler for SIGTSTP if it is not being ignored */
	if (sigaction(SIGTSTP, NULL, &sa) == -1)
		errmsg_exit1("sigaction - SIGTSTP failed, %s\n", ERR_MSG);

	if (sa.sa_handler != SIG_IGN) {
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		sa.sa_handler = tstp_handler;
		if (sigaction(SIGTSTP, &sa, NULL) == -1)
			errmsg_exit1("sigaction - SIGTSTP failed, %s\n",
				ERR_MSG);
	}

	while (1) {
		fprintf(stderr, "Enter Control-Z to send a SIGTSTP.\n");
		pause();
	}
}

static void
tstp_handler(int sig)
{
	int save_errno;
	sigset_t tstpmask, prevmask;
	struct sigaction sa;

	save_errno = errno;
	
	printf("Caught SIGTSTP (%d)\n", sig);

	/* Set handling to default */
	if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
		errmsg_exit1("signal - SIGTSTP failed, %s\n", ERR_MSG);

	/* Generate a further SIGTSTP */
	raise(SIGTSTP);

	/* 
	 * Unblock SIGTSTP;
	 * the pending SIGTSTP immediately suspends the program.
	 */
	sigemptyset(&tstpmask);
	sigaddset(&tstpmask, SIGTSTP);
	if (sigprocmask(SIG_UNBLOCK, &tstpmask, &prevmask) == -1)
		errmsg_exit1("sigprocmask - SIG_UNBLOCK failed, %s\n", ERR_MSG);

	/* Execution resumes here after SIGCONT */
	if (sigprocmask(SIG_SETMASK, &prevmask, NULL) == -1)
		errmsg_exit1("sigprocmask - SIG_SETMASK failed, %s\n", ERR_MSG);
	
	/* Reestablish handler */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = tstp_handler;
	if (sigaction(SIGTSTP, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGTSTP failed, %s\n", ERR_MSG);

	printf("Exiting SIGTSTP handler.\n");

	errno = save_errno;
}
