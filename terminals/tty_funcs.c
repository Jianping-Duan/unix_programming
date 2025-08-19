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
#include <termios.h>
#include <signal.h>
#include "tty_funcs.h"

static struct termios usrtos;

static void gene_handler(int);
static void tstp_handler(int);

int
main(int argc, char *argv[])
{
	struct sigaction sa, prev;
	char ch;
	ssize_t n;

	assert(argv[0] != NULL);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (argc <= 1) {	/* Use raw mode */
		if (tty_setraw(STDIN_FILENO, &usrtos) == -1)
			errmsg_exit1("tty_setraw failed\n");
		goto no_cbreak_mode;
	}

	if (tty_setcbreak(STDIN_FILENO, &usrtos) == -1)
		errmsg_exit1("tty_setcbreak failed\n");

	/*
	 * Terminal special characters can generate signals in cbreak mode.
	 * Catch them so that we can adjust the terminal mode. We establish
	 * handlers only if the signals are not being ignored.
	 */

	sa.sa_handler = gene_handler;

	if (sigaction(SIGQUIT, NULL, &prev) == -1)
		errmsg_exit1("sigaction (1) failed, %s\n", ERR_MSG);
	if (prev.sa_handler != SIG_IGN)
		if (sigaction(SIGQUIT, &sa, NULL) == -1)
			errmsg_exit1("sigaction (2) failed, %s\n", ERR_MSG);

	if (sigaction(SIGINT, NULL, &prev) == -1)
		errmsg_exit1("sigaction (3) failed, %s\n", ERR_MSG);
	if (prev.sa_handler != SIG_IGN)
		if (sigaction(SIGINT, &sa, NULL) == -1)
			errmsg_exit1("sigaction (4) failed, %s\n", ERR_MSG);

	sa.sa_handler = tstp_handler;

	if (sigaction(SIGTSTP, NULL, &prev) == -1)
		errmsg_exit1("sigaction (5) failed, %s\n", ERR_MSG);
	if (prev.sa_handler != SIG_IGN)
		if (sigaction(SIGTSTP, &sa, NULL) == -1)
			errmsg_exit1("sigaction (6) failed, %s\n", ERR_MSG);

no_cbreak_mode:

	sa.sa_handler = gene_handler;
	if (sigaction(SIGTERM, &sa, NULL) == -1)
		errmsg_exit1("sigaction (7) failed, %s\n", ERR_MSG);

	setbuf(stdout, NULL);	/* Disable stdout buffering */

	while (1) {		/* Read and echo stdin */
		if ((n = read(STDIN_FILENO, &ch, 1)) == -1) {
			fprintf(stderr, "read failed, %s\n", ERR_MSG);
			break;
		}

		if (n == 0)	/* Can occur after terminal disconnect */
			break;

		if (isalpha(ch))
			putchar(toupper(ch));
		else if (ch == '\n' || ch == '\r')
			putchar(ch);
		else if (iscntrl(ch))
			printf("^%c", ch ^ 64);	/* Echo Control-A as ^A, etc. */
		else
			putchar('*');		/* All other chars as '*' */

		if (ch == '$' || ch == '@')	/* Quit loop */
			break;
	}

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usrtos) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}

static void
gene_handler(int sig)
{
	assert(sig == SIGQUIT || sig == SIGINT || sig == SIGTERM);

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usrtos) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);
	_exit(EXIT_SUCCESS);
}

static void
tstp_handler(int sig)
{
	struct termios tos;
	sigset_t tstpmask, pmask;
	struct sigaction sa;
	int save_errno;

	assert(sig == SIGTSTP);

	save_errno = errno;

	/* 
	 * Save current terminal settings, restore terminal to state at time of
	 * program startup.
	 */

	if (tcgetattr(STDIN_FILENO, &tos) == -1)
		errmsg_exit1("tcgetattr failed, %s\n", ERR_MSG);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usrtos) == -1)
		errmsg_exit1("tcsetaatr failed, %s\n", ERR_MSG);

	/* 
	 * Set the disposition of SIGTSTP to the default, raise the signal once
	 * more, and then unblock it so that we actually stop
	 */

	if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
		errmsg_exit1("signal failed, %s\n", ERR_MSG);
	raise(SIGTSTP);

	sigemptyset(&tstpmask);
	sigaddset(&tstpmask, SIGTSTP);
	if (sigprocmask(SIG_UNBLOCK, &tstpmask, &pmask) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);

	/* Execution resumes here after SIGCONT */

	if (sigprocmask(SIG_SETMASK, &pmask, NULL) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);

	sigemptyset(&sa.sa_mask);	/* Reestablish handler */
	sa.sa_handler = tstp_handler;
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGTSTP, &sa, NULL) == -1)
		errmsg_exit1("sigaction (9) failed, %s\n", ERR_MSG);

	/*
	 * The user may have changed the terminal settings while we were
	 * stopped; save the settings so we can restore them later.
	 */

	if (tcgetattr(STDIN_FILENO, &usrtos) == -1)
		errmsg_exit1("tcgetattr failed, %s\n", ERR_MSG);

	/* Restore our terminal settings */
	
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tos) == -1)
		errmsg_exit1("tcsetattr failed, %s\n", ERR_MSG);

	errno = save_errno;
}
