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
#include <sys/stat.h>
#include <time.h>	/* time(), localtime(), strftime() */

union semun {
	int		val;	/* value for SETVAL */
        struct semid_ds	*buf;	/* buffer for IPC_STAT & IPC_SET */
        unsigned short	*array; /* array for GETALL & SETALL */
};

static char * currtime(const char *);

int
main(int argc, char *argv[])
{
	int semid;
	union semun arg;
	struct sembuf sop;

	if (argc < 2 || argc > 3 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s init-value\n\tor: %s semid opt\n",
			argv[0], argv[0]);

	if (argc == 2) {	/* Create and initialize semaphore */
		/*
		 * Based on the values of key and flag, semget() returns the
		 * identifier of a newly created or previously existing set of
		 * semaphores. The key is analogous to a filename: it provides a
		 * handle that names an IPC object. There are three ways to
		 * specify a key:
		 *
		 * IPC_PRIVATE may be specified, in which case a new IPC object
		 * will be created.
		 *
		 * An integer constant may be specified. If no IPC object
		 * corresponding to key is specified and the IPC_CREAT bit is
		 * set in flag, a new one will be created.
		 *
		 * The ftok(3) function may be used to generate a key from a
		 * pathname.
		 *
		 * If a new set of semaphores is being created, nsems is used to
		 * indicate the number of semaphores the set should contain.
		 * Otherwise, nsems may be specified as 0.
		 */
		if ((semid = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR)) == -1)
			errmsg_exit1("semget failed, %s\n", ERR_MSG);

		arg.val = getint(argv[1]);
		/*
		 * The semctl() system call performs the operation indicated by
		 * cmd on the semaphore set indicated by semid. A fourth
		 * argument, a union semun arg, is required for certain values
		 * of cmd. For the commands that use the arg argument, union
		 * semun must be defined.
		 *
		 * Non-portable software may define _WANT_SEMUN before including
		 * sys/sem.h to use the system definition of union semun.
		 *
		 * Commands are performed as follows:
		 *
		 * IPC_STAT     Fetch the semaphore set's struct semid_ds,
		 *		storing it in the memory pointed to by arg.buf.
		 *
		 * IPC_SET      Changes the sem_perm.uid, sem_perm.gid, and
		 *		sem_perm.mode members of the semaphore set's
		 *		struct semid_ds to match those of the struct
		 *		pointed to by arg.buf. The calling process's
		 *		effective uid must match either sem_perm.uid or
		 *		sem_perm.cuid, or it must have superuser
		 *		privileges.
		 *
		 * IPC_RMID     Immediately removes the semaphore set from the
		 *		system. The calling process's effective uid must
		 *		equal the semaphore set's sem_perm.uid or
		 *		sem_perm.cuid, or the process must have
		 *		superuser privileges.
		 *
		 * GETVAL       Return the value of semaphore number semnum.
		 *
		 * SETVAL       Set the value of semaphore number semnum to
		 *		arg.val. Outstanding adjust on exit values for
		 *		this semaphore in any process are cleared.
		 *
		 * GETPID       Return the pid of the last process to perform an
		 *		operation on semaphore number semnum.
		 *
		 * GETNCNT      Return the number of processes waiting for
		 *		semaphore number semnum's value to become
		 *		greater than its current value.
		 *
		 * GETZCNT      Return the number of processes waiting for
		 *		semaphore number semnum's value to become 0.
		 *
		 * GETALL       Fetch the value of all of the semaphores in the
		 *		set into the array pointed to by arg.array.
		 *
		 * SETALL       Set the values of all of the semaphores in the
		 *		set to the values in the array pointed to by
		 *		arg.array. Outstanding adjust on exit values for
		 *		all semaphores in this set, in any process are
		 *		cleared.
		 */
		if (semctl(semid, 0, SETVAL, arg) == -1)
			errmsg_exit1("semctl failed, %s\n", ERR_MSG);

		printf("Semaphore ID = %d\n", semid);
	} else {	/* Perform an operation on first semaphore */
		semid = getint(argv[1]);

		sop.sem_num = 0;	/* Specifies first semaphore in set */
		sop.sem_op = getint(argv[2]);	/* Add, substract or 0 */
		sop.sem_flg = 0;	/* No special options for operation */

		printf("%d: about to semop at %s\n", getpid(), currtime("%T"));
		/*
		 * The semop() system call atomically performs the array of
		 * operations indicated by array on the semaphore set indicated
		 * by semid. The length of array is indicated by nops. Each
		 * operation is encoded in a struct sembuf.
		 *
		 * For each element in array, sem_op and sem_flg determine an
		 * operation to be performed on semaphore number sem_num in the
		 * set. The values SEM_UNDO and IPC_NOWAIT may be OR'ed into the
		 * sem_flg member in order to modify the behavior of the given
		 * operation.
		 *
		 * More the value of sem_op, see semop(2)
		 */
		if (semop(semid, &sop, 1) == -1)
			errmsg_exit1("semop failed, %s\n", ERR_MSG);
		printf("%d: semop completed at %s\n", getpid(), currtime("%T"));
	}

	exit(EXIT_SUCCESS);
}


static char *
currtime(const char *fmt)
{
#define BUFSIZE	512
	static char buf[BUFSIZE];
	time_t tim;
	size_t sz;
	struct tm *tmptr;

	tim = time(NULL);
	if ((tmptr = localtime(&tim)) == NULL) {
		fprintf(stderr, "localtime failed, %s\n", ERR_MSG);
		return NULL;
	}

	/* more formats see strftime(3) */
	sz = strftime(buf, BUFSIZE, (fmt != NULL) ? fmt : "%c", tmptr);	
	return ((sz == 0) ? NULL : buf);
}
