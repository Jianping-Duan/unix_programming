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
#include <sys/stat.h>
#include <fcntl.h>

static char * file_perms(mode_t, int);

#define FPERMS	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#define UMASKSTR	(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int
main(int argc, char *argv[])
{
	int fd;
	struct stat st;
	mode_t u;

	if (argc != 2)
		errmsg_exit1("Usage: %s file\n", argv[0]);

	if ((fd = open(argv[1], O_RDWR | O_CREAT | O_EXCL, FPERMS)) == -1)
		errmsg_exit1("open file %s failure, %s\n", argv[1], ERR_MSG);

	u = umask(0);	/* Retrieves (and clears) umask value */

	if (stat(argv[1], &st) == -1)
		errmsg_exit1("stat failure, %s\n", ERR_MSG);
	printf("Requested file perms: %s\n", file_perms(FPERMS, 0));
    printf("Process umask:        %s\n", file_perms(u, 0));
    printf("Actual file perms:    %s\n\n", file_perms(st.st_mode, 0));

	printf("remove this file.\n");
	if (remove(argv[1]) == -1)
		errmsg_exit1("remove file %s failure, %s\n", argv[1], ERR_MSG);

	u = umask(UMASKSTR);

	printf("create a empty file:\n");
	if (creat(argv[1], FPERMS) == -1)
		errmsg_exit1("create file %s failure, %s\n", argv[1], ERR_MSG);

	if (stat(argv[1], &st) == -1)
		errmsg_exit1("stat failure, %s\n", ERR_MSG);
	printf("Requested file perms: %s\n", file_perms(FPERMS, 0));
    printf("Process umask:        %s\n", file_perms(u, 0));
    printf("Actual file perms:    %s\n\n", file_perms(st.st_mode, 0));
	
	exit(EXIT_SUCCESS);
}

/* Return ls(1)-style string for file permissions mask */
static char *
file_perms(mode_t perm, int flags)
{
	static char permstr[10];
/* 
 * Include set-user-ID, set-group-ID, and sticky
 * bit information in returned string.
 */
#define FP_SPECIAL	0x01

	/* 
	 * If FP_SPECIAL was specified, we emulate the trickery of ls(1) in
	 * returning set-user-ID, set-group-ID, and sticky bit information in
	 * the user/group/other execute fields. This is made more complex by
	 * the fact that the case of the character displayed for this bits
	 * depends on whether the corresponding execute bit is on or off.
	 */
	sprintf(permstr, "%c%c%c%c%c%c%c%c%c",
		(perm & S_IRUSR) ? 'r' : '-', (perm & S_IWUSR) ? 'w' : '-',
		(perm & S_IXUSR) ? 
			(((perm & S_ISUID) && (flags & FP_SPECIAL) ? 's' : 'x')) :
			(((perm & S_ISUID) && (flags & FP_SPECIAL) ? 'S' : '-')),
		(perm & S_IRGRP) ? 'r' : '-', (perm & S_IWGRP) ? 'w' : '-',
		(perm & S_IXGRP) ?
			(((perm & S_ISGID) && (flags & FP_SPECIAL) ? 's' : 'x')) :
			(((perm & S_ISGID) && (flags & FP_SPECIAL) ? 'S' : '-')),
		(perm & S_IROTH) ? 'r' : '-', (perm & S_IWOTH) ? 'w' : '-',
		(perm & S_IXOTH) ?
			(((perm & S_ISVTX) && (flags & FP_SPECIAL) ? 't' : 'x')) :
			(((perm & S_ISVTX) && (flags & FP_SPECIAL) ? 'T' : '-'))
	);

	return permstr;
}
