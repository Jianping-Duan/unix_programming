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
	long tsfrs = 0, bytes = 0, cnt;
	struct shmseg *seg;

	/* Get IDs for semaphore set and shared memory created by writer */

	if ((semid = semget(SYSV_SEM_KEY, 0, 0)) == -1)
		errmsg_exit1("semget failed, %s\n", ERR_MSG);

	if ((shmid = shmget(SYSV_SHM_KEY, 0, 0)) == -1)
		errmsg_exit1("shmget failed, %s\n", ERR_MSG);

	/* Attach shared memory read-only, as we will only read */

	seg = (struct shmseg *)shmat(shmid, NULL, SHM_RDONLY);
	if (seg == (void *)-1)
		errmsg_exit1("shmat failed, %s\n", ERR_MSG);

	/* Transfer blocks of data from shared memory to stdout */

	while (1) {
		if (svsem_wait(semid, SYSV_READ_SEM) == -1)
			errmsg_exit1("svsem_wait failed\n");

		if (seg->shms_cnt == 0)		/* Writer encountered EOF */
			break;

		cnt = write(STDOUT_FILENO, seg->shms_buf, seg->shms_cnt);
		if (cnt != seg->shms_cnt)
			errmsg_exit1("write (%ld) failed, %s\n", cnt, ERR_MSG);

		if (svsem_post(semid, SYSV_WRITE_SEM) == -1)
			errmsg_exit1("svsem_post failed\n");

		bytes += seg->shms_cnt;
		tsfrs++;
	}
	
	if (shmdt(seg) == -1)
		errmsg_exit1("shmdt failed, %s\n", ERR_MSG);

	/* Give writer one more turn, so it can clean up */
	if (svsem_post(semid, SYSV_WRITE_SEM) == -1)
		errmsg_exit1("svsem_post failed\n");

	fprintf(stderr, "Received %ld bytes (%ld transfers)\n", bytes, tsfrs);

	exit(EXIT_SUCCESS);
}
