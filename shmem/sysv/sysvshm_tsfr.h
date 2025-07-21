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
#ifndef _SYSVSHM_TSFR_H_
#define _SYSVSHM_TSFR_H_

#define SYSV_SHM_KEY	0x1234		/* Key for shared memory segment */
#define SYSV_SEM_KEY	0x2345		/* Key for semaphore set */

/* Permissions for our IPC objects */
#define IPC_OBJ_PERMS	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/*
 * Two semaphores are used to ensure exclusive, alternating access to the
 * shared memory segment
 */
#define SYSV_WRITE_SEM	0	/* Writer has access to shared memory */
#define SYSV_READ_SEM	1	/* Reader has access to shared memory */

union semun {
	int		val;	/* value for SETVAL */
        struct semid_ds	*buf;	/* buffer for IPC_STAT & IPC_SET */
        unsigned short	*array; /* array for GETALL & SETALL */
};

struct shmseg {
	long	shms_cnt;		/* # bytes used in 'shms_buf' */
	char	shms_buf[BUF_SIZE];	/* Data being transferred */
};


static inline int
svsem_avai_init(int semid, int semnum)
{
	union semun arg;

	arg.val = 1;
	return semctl(semid, semnum, SETVAL, arg);
}

static inline int
svsem_inuse_init(int semid, int semnum)
{
	union semun arg;

	arg.val = 0;
	return semctl(semid, semnum, SETVAL, arg);
}

static int
svsem_wait(int semid, int semnum)
{
	struct sembuf sops;

	sops.sem_num = semnum;
	sops.sem_op = -1;
	sops.sem_flg = 0;

	while (semop(semid, &sops, 1) == -1)
		if (errno != EINTR)
			return -1;

	return 0;
}

static int
svsem_post(int semid, int semnum)
{
	struct sembuf sops;

	sops.sem_num = semnum;
	sops.sem_op = 1;
	sops.sem_flg = 0;

	return semop(semid, &sops, 1);
}


#endif	/* !_SYSVSHM_TSFR_H_ */
