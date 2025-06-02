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

static void exit_fun1(void);
static void exit_fun2(void);

int
main(void)
{
	/*
	 * The atexit() function registers the given function to be called at
	 * program exit, whether via exit(3) or via return from the program's
	 * main(). Functions so registered are called in reverse order; no
	 * arguments are passed.
	 *
	 * These functions must not call exit(); if it should be necessary to
	 * terminate the process while in such a function, the _exit(2) function
	 * should be used. (Alternatively, the function may cause abnormal process
	 * termination, for example by calling abort(3).)
	 *
	 * The atexit() function returns the value 0 if successful; otherwise the
	 * value -1 is returned and the global variable errno is set to indicate the
	 * error.
	 */
	if (atexit(exit_fun1) != 0)
		errmsg_exit1("atexit - exit_fun1 failed, %s\n", ERR_MSG);
	if (atexit(exit_fun2) != 0)
		errmsg_exit1("atexit - exit_fun2 failed, %s\n", ERR_MSG);
	
	exit(3);
}

static void
exit_fun1(void)
{
	printf("atexit function 1 called.\n");
}

static void
exit_fun2(void)
{
	printf("atexit function 2 called.\n");
}
