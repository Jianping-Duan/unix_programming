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
/*
 * Using fork() + wait() to control the memory footprint of an application.
 * 
 * This program contains a function that (artificially) consumes a large
 * amount of memory. To avoid changing the process's memory footprint, the
 * program creates a child process that calls the function. When the child
 * terminates, all of its memory is freed, and the memory consumption of
 * the parent is left unaffected.
 */
#include "unibsd.h"
#include <sys/wait.h>

static int func(int);

int
main(int argc, char *argv[])
{
	int arg, status;
	pid_t pid;

	setbuf(stdout, NULL);	/*  Disable buffering of stdout */
	arg = (argc > 1) ? getint(argv[1]) : 0;

	printf("Program break in parent: %10p\n", sbrk(0));

	if ((pid = fork()) == -1)
		errmsg_exit1("fork failed, %s\n", ERR_MSG);

	if (pid == 0)
		exit(func(arg));	/* uses return value as exit status */

	/*
	 * Parent waits for child to terminate. 
	 * It can determine the result of func() by inspecting 'status'
	 */
	if (wait(&status) == -1)
		errmsg_exit1("wait failed, %s\n", ERR_MSG);

	printf("Program break in parent: %10p\n", sbrk(0));
	printf("status = %d (%d)\n", status, WEXITSTATUS(status));

	exit(EXIT_SUCCESS);
}

static int
func(int arg)
{
	int i;

	for (i = 0; i < 0x100; i++)
		if (malloc(0x8000) == NULL)
			errmsg_exit1("malloc failed, %s\n", ERR_MSG);

	return arg;
}
