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
#include <sys/sem.h>

union semun {
	int		val;	/* value for SETVAL */
        struct semid_ds	*buf;	/* buffer for IPC_STAT & IPC_SET */
        unsigned short	*array; /* array for GETALL & SETALL */
};

int
main(int argc, char *argv[])
{
	int semid, i;
	union semun arg;
	struct semid_ds sds;

	if (argc < 3 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s semid value...\n", argv[0]);

	semid = getint(argv[1]);

	/* Obtain size of semaphore set */

	arg.buf = &sds;
	if (semctl(semid, 0, IPC_STAT, arg) == -1)
		errmsg_exit1("semctl - IPC_STAT failed, %s\n", ERR_MSG);

	/*
	 * The number of values supplied on the command line must match the
	 * number of semaphores in the set
	 */

	if (sds.sem_nsems != argc - 2)
		errmsg_exit1("Set contains %d semaphores, but %d values were "
			"supplied\n", sds.sem_nsems, argc - 2);

	/* Set up array of values; perform semaphore initialization */

	arg.array = calloc(sds.sem_nsems, sizeof(*arg.array));
	for (i = 2; i < argc; i++)
		arg.array[i - 2] = getint(argv[i]);

	if (semctl(semid, 0, SETALL, arg) == -1)
		errmsg_exit1("semctl - SETALL failed, %s\n", ERR_MSG);
	printf("Semaphore values changed (PID=%d)\n", getpid());
	
	exit(EXIT_SUCCESS);
}
