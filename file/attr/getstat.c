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
#include <sys/types.h>	/* major(), minor() */
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>	/* ctime() */

static void display_statinfo(const struct stat *);
static char * file_perms(mode_t, int);

int
main(int argc, char *argv[])
{
	struct stat st;

	if (argc < 2 || argc > 3) {
		errmsg_exit1("Usage: %s file [-l]\n"
			"-l = use lstat() instend of stat().\n", argv[0]);
	}

	if (argc == 2) {
		if (stat(argv[1], &st) == -1)
			errmsg_exit1("stat failure, %s\n", ERR_MSG);
	} else {
		if (lstat(argv[1], &st) == -1)
			errmsg_exit1("lstat failure, %s\n", ERR_MSG);
	}

	display_statinfo(&st);
	
	return 0;
}

static void
display_statinfo(const struct stat *st)
{
	printf("File type:\t\t\t");
	switch (st->st_mode & S_IFMT) {
	case S_IFREG:
		printf("regular file.\n");
		break;
	case S_IFDIR:
		printf("directory.\n");
		break;
	case S_IFCHR:
		printf("character device.\n");
		break;
	case S_IFBLK:
		printf("block device.\n");
		break;
	case S_IFLNK:
		printf("symbol (soft) link.\n");
		break;
	case S_IFIFO:
		printf("FIFO or pipe.\n");
		break;
	case S_IFSOCK:
		printf("socket.\n");
	default:
		printf("unknown file type.\n");
	}

	printf("Dvice containing i-node:\tmajor = %d, minor = %d\n",
		major(st->st_dev), minor(st->st_dev));
	printf("I-node number:\t\t\t%lu\n", st->st_ino);
	printf("Mode:\t\t\t\t%d (%s)\n", st->st_mode,
		file_perms(st->st_mode, 0));

	if (st->st_mode & (S_ISUID | S_ISGID | S_ISVTX)) {
		printf("speical bits set:\t%s%s%s\n",
			(st->st_mode & S_ISUID) ? "set-UID" : "",
			(st->st_mode & S_ISGID) ? "set-GID" : "",
			(st->st_mode & S_ISVTX) ? "sticky " : ""
		);
	}

	printf("Number of (hard) links:\t\t%lu\n", st->st_nlink);
	printf("Ownership:\t\t\tUID=%d GID=%d\n", st->st_uid, st->st_gid);

	if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode)) {
		printf("Device number(st_dev):\t\tmajor=%d, minor=%d\n",
			major(st->st_rdev), minor(st->st_rdev));
	}

	printf("File size:\t\t\t%ld bytes\n", st->st_size);
	printf("Optimal I/O block size:\t\t%d bytes\n", st->st_blksize);
	printf("512B blocks allocated:\t\t%ld\n", st->st_blocks);
	printf("Last file access:\t\t%s", ctime(&(st->st_atime)));
	printf("Last file modification:\t\t%s", ctime(&(st->st_mtime)));
	printf("Last status change:\t\t%s", ctime(&(st->st_ctime)));

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
			(((perm & S_ISUID) && (flags & FP_SPECIAL) ?
			's' : 'x')) :
			(((perm & S_ISUID) && (flags & FP_SPECIAL) ?
			'S' : '-')),
		(perm & S_IRGRP) ? 'r' : '-', (perm & S_IWGRP) ? 'w' : '-',
		(perm & S_IXGRP) ?
			(((perm & S_ISGID) && (flags & FP_SPECIAL) ?
			's' : 'x')) :
			(((perm & S_ISGID) && (flags & FP_SPECIAL) ?
			'S' : '-')),
		(perm & S_IROTH) ? 'r' : '-', (perm & S_IWOTH) ? 'w' : '-',
		(perm & S_IXOTH) ?
			(((perm & S_ISVTX) && (flags & FP_SPECIAL) ?
			't' : 'x')) :
			(((perm & S_ISVTX) && (flags & FP_SPECIAL) ?
			'T' : '-'))
	);

	return permstr;
}
