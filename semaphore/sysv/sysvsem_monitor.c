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
#include <time.h>	/* ctime() */

union semun {
	int		val;	/* value for SETVAL */
        struct semid_ds	*buf;	/* buffer for IPC_STAT & IPC_SET */
        unsigned short	*array; /* array for GETALL & SETALL */
};

int
main(int argc, char *argv[])
{
	struct semid_ds sds;
	union semun arg;
	int semid, i;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s semid\n", argv[0]);

	semid = getint(argv[1]);

	arg.buf = &sds;
	if (semctl(semid, 0, IPC_STAT, arg) == -1)
		errmsg_exit1("semctl - IPC_STAT failed, %s\n", ERR_MSG);

	printf("Semaphore changed:\t%s", ctime(&sds.sem_ctime));
	printf("Lsat semop():\t\t%s", ctime(&sds.sem_otime));

	/* Display per-semaphore information */

	/*
	 * On success, when cmd is one of GETVAL, GETPID, GETNCNT or GETZCNT,
	 * semctl() returns the corresponding value; otherwise, 0 is returned.
	 * On failure, -1 is returned, and errno is set to indicate the error.
	 */
	arg.array = xcalloc(sds.sem_nsems, sizeof(*arg.array));
	if (semctl(semid, 0, GETALL, arg) == -1)
		errmsg_exit1("semctl - GETALL failed, %s\n", ERR_MSG);

	printf("Sem #\tValue\tSEMPID\tSEMNCNT\tSEMZCNT\n");
	for (i = 0; i < sds.sem_nsems; i++)
		printf("%-3d\t%-5d\t%-5d\t%-5d\t%-5d\n", i, arg.array[i],
			semctl(semid, i, GETPID), semctl(semid, i, GETNCNT),
			semctl(semid, i, GETZCNT));
	
	xfree(arg.array);

	exit(EXIT_SUCCESS);
}

