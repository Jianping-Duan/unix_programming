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
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include "sysvshm_tsfr.h"

int
main(void)
{
	int semid, shmid;
	long tsfrs = 0, bytes = 0;
	struct shmseg *seg;

	/*
	 * Create set containing two semaphores; initialize so that writer has
	 * first access to shared memory.
	 */

	if ((semid = semget(SYSV_SEM_KEY, 2, IPC_CREAT | IPC_OBJ_PERMS)) == -1)
		errmsg_exit1("semget failed, %s\n", ERR_MSG);

	if (svsem_avai_init(semid, SYSV_WRITE_SEM) == -1)
		errmsg_exit1("svsem_avai_init failed\n");
	if (svsem_inuse_init(semid, SYSV_READ_SEM) == -1)
		errmsg_exit1("svsem_inuse_init failed\n");

	/* Create shared memory; attach at address chosen by system */

	shmid = shmget(SYSV_SHM_KEY, sizeof(struct shmseg),
		IPC_CREAT | IPC_OBJ_PERMS);
	if (shmid == -1)
		errmsg_exit1("shmget failed, %s\n", ERR_MSG);

	/*
	 * The shmat() system call attaches the shared memory segment identified
	 * by shmid to the calling process's address space.  The address where
	 * the segment is attached is determined as follows:
	 *
	 *	If addr is 0, the segment is attached at an address selected by
	 *	the kernel.
	 *
	 *	If addr is nonzero and SHM_RND is not specified in flag, the
	 *	segment is attached the specified address.
	 *
	 *	If addr is specified and SHM_RND is specified, addr is rounded
	 *	down to the nearest multiple of SHMLBA.
	 *
	 * If the SHM_REMAP flag is specified and the passed addr is not NULL,
	 * any existing mappings in the virtual addresses range are cleared
	 * before the segment is attached. If the flag is not specified, addr is
	 * not NULL, and the virtual address range contains some pre-existing
	 * mappings, the shmat() call fails.
	 *
	 *
	 */
	if ((seg = (struct shmseg *)shmat(shmid, NULL, 0)) == (void *)-1)
		errmsg_exit1("shmat failed, %s\n", ERR_MSG);

	/* Transfer blocks of data from stdin to shared memory */

	while (1) {
		/* Wait for our turn */
		if (svsem_wait(semid, SYSV_WRITE_SEM) == -1)
			errmsg_exit1("svsem_wait (1) failed\n");

		seg->shms_cnt = read(STDIN_FILENO, seg->shms_buf, BUF_SIZE);
		if (seg->shms_cnt == -1)
			errmsg_exit1("read failed, %s\n", ERR_MSG);

		/* Give reader a turn */
		if (svsem_post(semid, SYSV_READ_SEM) == -1)
			errmsg_exit1("svsem_post failed\n");

		if (seg->shms_cnt == 0)
			break;

		tsfrs++;
		bytes += seg->shms_cnt;
	}

	/*
	 * Wait until reader has let us have one more turn. We then know reader
	 * has finished, and so we can delete the IPC objects.
	 */
	if (svsem_wait(semid, SYSV_WRITE_SEM) == -1)
		errmsg_exit1("svsem_wait (2) failed\n");

	if (semctl(semid, 0, IPC_RMID) == -1)
		errmsg_exit1("semctl - IPC_RMID failed, %s\n", ERR_MSG);
	if (shmdt(seg) == -1)
		errmsg_exit1("shmdt failed, %s\n", ERR_MSG);
	if (shmctl(shmid, IPC_RMID, NULL))
		errmsg_exit1("shmctl - IPC_RMID failed, %s\n", ERR_MSG);

	fprintf(stderr, "Send %ld bytes (%ld transfers)\n", bytes, tsfrs);

	exit(EXIT_SUCCESS);
}
