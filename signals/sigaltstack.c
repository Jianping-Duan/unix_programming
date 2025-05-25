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

static void sigsegv_handler(int);
static _Noreturn void overflow_stack(int);

int
main(void)
{
	stack_t sigstack;	/* more details see sigaltstack(2) */
	struct sigaction sa;
	int noinit;
	void *heapused;

	printf("Top of standard stack is near %10p\n", (void *)&noinit);

	/*
	 * If SS_DISABLE is set in ss_flags, ss_sp and ss_size are ignored and the
	 * signal stack will be disabled.  A disabled stack will cause all signals
	 * to be taken on the regular user stack.  If the stack is later re-enabled
	 * then all signals that were specified to be processed on an alternate
	 * stack will resume doing so.
	 *
	 * If oss is non-zero, the current signal stack state is returned.  The
	 * ss_flags field will contain the value SS_ONSTACK if the thread is
	 * currently on a signal stack and SS_DISABLE if the signal stack is
	 * currently disabled.
	 * 
	 */
	sigstack.ss_sp = xmalloc(SIGSTKSZ);
	sigstack.ss_size = SIGSTKSZ;
	sigstack.ss_flags = 0;
	if (sigaltstack(&sigstack, NULL) == -1)
		errmsg_exit1("sigaltstack failed, %s\n", ERR_MSG);

	/*
	 * The sbrk() function raises the break by incr bytes, thus allocating at
	 * least incr bytes of new memory in the data segment.
	 * If incr is negative, the break is lowered by incr bytes.
	 *
	 * sbrk() is sometimes used to monitor heap use by calling with an argument
	 * of 0. The result is unlikely to reflect actual utilization in 
	 * combination with an mmap(2) based malloc.
	 */
	if ((heapused = sbrk(0)) == (void *)-1)
		errmsg_exit1("sbrk(0) failed, %s\n", ERR_MSG);
	printf("Alternate stack is at\t%10p-%p", sigstack.ss_sp, heapused - 1);

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sigsegv_handler;
	sa.sa_flags = SA_ONSTACK;	 /* Handler uses alternate stack */
	if (sigaction(SIGSEGV, &sa, NULL) == -1)
		errmsg_exit1("sigaction failed, %s\n", ERR_MSG);

	overflow_stack(1);
	
	exit(EXIT_SUCCESS);
}

static void
sigsegv_handler(int sig)
{
	int noinit;

	printf("Caught signal %d (%s)\n", sig, strsignal(sig));
	printf("Top of Handler stack near\t%10p\n", (void *)&noinit);
	fflush(NULL);

	exit(EXIT_FAILURE);	/* Can't return after SIGSEGV */
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Winfinite-recursion"
#elif defined(__GCC__)
#pragma clang diagnostic ignored "-Winfinite-recursion"
#endif
static _Noreturn void
overflow_stack(int callnum)
{
	char a[102400];

	printf("Call %4d - top of stack near %10p\n", callnum, &a[0]);
	overflow_stack(callnum + 1);
}
