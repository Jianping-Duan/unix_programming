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
#include <setjmp.h>

static enum jmp_typ {SIGLONGJMP = 0, LONGJMP} jmpflag;

static jmp_buf env;
static sigjmp_buf senv;

/* Set to 1 once "env" buffer has been initialized by [sig]setjmp() */
static volatile sig_atomic_t canjmp = 0;

static void sig_handler(int);
static void print_sigset(const sigset_t *);
static void print_sigmask(const char *);

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	int flag;

	if (argc != 2) {
		errmsg_exit1("Usage: %s jmpflag\n"
			"\t0: siglongjmp; 1: longjmp\n", argv[0]);
	}
	
	flag = (int)getlong(argv[1], GN_ANY_BASE);
	if (flag != SIGLONGJMP && flag != LONGJMP) {
		fprintf(stderr, "Invalid argument, %s\n", argv[1]);
		errmsg_exit1("Usage: %s jmpflag\n"
			"\t0: siglongjmp; 1: longjmp\n", argv[0]);
	}
	jmpflag = (flag == 0) ? SIGLONGJMP : LONGJMP;

	print_sigmask("Signal mask at startup:\n");

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	if (jmpflag == SIGLONGJMP) {
		printf("Calling siglongjmp()\n");
		/*
		 * The sigsetjmp()/siglongjmp() function pairs save and restore
		 * the signal mask if the argument savemask is non-zero,
		 * otherwise only the register set and the stack are saved.
		 * More details see sigsetjmp(3)
		 */
		if (sigsetjmp(senv, 1) == 0)
			canjmp = 1;	/* Executed after [sig]setjmp() */
		else	/* Executed after [sig]longjmp() */
			print_sigmask("After jump from handler, "
				"signal mask is:\n");
	} else {
		printf("Calling longjmp()\n");
		if (setjmp(env) == 0)
			canjmp = 1;
		else
			print_sigmask("After jump from handler, "
				"signal mask is:\n");
	}

	while (1) {	/* Wait for signals until killed */
		printf("Please enter Control-C to test or enter Control-\\ "
			"to quit!\n");
		pause();
	}
	
	exit(EXIT_SUCCESS);
}

static void
sig_handler(int sig)
{
	printf("Received signal %d (%s), signal mask is:\n", sig,
		strsignal(sig));
	print_sigmask(NULL);

	if (!canjmp) {
		printf("'env' buffer not yet set, doing a simple return.\n");
		return;
	}

	if (jmpflag == SIGLONGJMP)
		siglongjmp(senv, 1);
	else
		longjmp(env, 1);
}

static void
print_sigset(const sigset_t *sigs)
{
	int sig, cnt = 0;

	for (sig = 1; sig < NSIG; sig++)
		if (sigismember(sigs, sig)) {
			cnt++;
			printf("\t\t%d (%s)\n", sig, strsignal(sig));
		}

	if (cnt == 0)
		printf("\t\t<empty signal set>\n");
}

/* Print mask of blocked signals for this process */
static void 
print_sigmask(const char *msg)
{
	sigset_t currmask;

	if (msg != NULL)
		printf("%s", msg);

	/*
	 * SIG_BLOCK
	 *	The new mask is the union of the current mask and the specified
	 *	set.
	 *
	 * SIG_UNBLOCK
	 *	The new mask is the intersection of the current mask and the
	 *	complement of the specified set.
	 * 
	 * SIG_SETMASK
	 *	The current mask is replaced by the specified set.
	 *
	 * If oset is not null, it is set to the previous value of the signal
	 * mask. When set is null, the value of how is insignificant and the
	 * mask remains unset providing a way to examine the signal mask without
	 * modification.
	 */
	if (sigprocmask(SIG_BLOCK, NULL, &currmask) == -1)
		errmsg_exit1("sigprocmask failed, %s\n", ERR_MSG);
	print_sigset(&currmask);
}
