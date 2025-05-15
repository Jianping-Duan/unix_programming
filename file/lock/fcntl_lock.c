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

#define MAX_LINE	128

static int mdlock_enabled(int);
static void display(int, char **, const int *);

int
main(int argc, char *argv[])
{
	int fd, *fdset, filenum;
	int i, nrd, cmd, status;
	char line[MAX_LINE];
	char cmdch, lock, whence;
	long st, len;
	struct flock fl;

	if (argc < 2)
		errmsg_exit1("Usage: %s file...\n", argv[0]);

	fdset = xmalloc(argc * sizeof(int));
	for (i = 1; i < argc; i++)
		if ((fdset[i] = open(argv[i], O_RDWR)) == -1)
			errmsg_exit1("open file %s failed, %s\n", argv[i], ERR_MSG);

	/* Inform user what type of locking is in effect for each file. */
	printf("File\t\tLocking\n");
	printf("---\t\t-------\n");

	for (i = 1; i < argc; i++) {
		printf("%-10s %s\n", argv[i], mdlock_enabled(fdset[i])
			? "mandatory" : "advisory");
	}
	printf("\n");

	printf("Enter ? for help\n");
	while (1) {	/* Prompt for locking command and carry it out */
		printf("PID=%d> ", getpid());
		fflush(stdout);

		if (fgets(line, MAX_LINE, stdin) == NULL)	/* EOF */
			break;
		line[strlen(line) - 1] = '\0';	/* Remove trailing '\n' */

		if (*line == '\0')
			continue;	/* Skip blank lines */

		if (line[0] == '?') {
			display(argc, argv, fdset);
			continue;
		}

		if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0)
			break;

		whence = 's';	 /* In case not otherwise filled in */
		
		if (argc == 2) {	/* Just 1 file arg on command line? */
			filenum = 1;	/* Then no need to read a file number */
			nrd = sscanf(line, "%c %c %ld %ld %c",
				&cmdch, &lock, &st, &len, &whence);
		} else {
			nrd = sscanf(line, "%d %c %c %ld %ld %c",
				&filenum, &cmdch, &lock, &st, &len, &whence);
		}

		if (filenum < 1 || filenum >= argc) {
			printf("File number must be in range 1 and %d\n", argc - 1);
			continue;
		}

		if (!((nrd >= 4 && argc == 2) || (nrd >= 5 && argc > 2)) ||
			strchr("gsw", cmdch) == NULL || strchr("rwu", lock) == NULL ||
			strchr("sce", whence) == NULL) {
			printf("Invalid command!\n");
			continue;
		}

		/* 
		 * F_GETLK:
		 * Get the first lock that blocks the lock description pointed to
		 * by the third argument, arg, taken as a pointer to a struct flock.
		 * The information retrieved overwrites the information passed to 
		 * fcntl() in the flock structure. If no lock is found that would
		 * prevent this lock from being created, the structure is left
		 * unchanged by this system call except for the lock type which is set
		 * to F_UNLCK.
		 *
		 * F_SETLK:
		 * Set or clear a file segment lock according to the lock
		 * description pointed to by the third argument, arg, taken as a
		 * pointer to a struct flock.  F_SETLK is used to establish shared
		 * (or read) locks (F_RDLCK) or exclusive (or write) locks, (F_WRLCK),
		 * as well as remove either type of lock (F_UNLCK). If a shared or
		 * exclusive lock cannot be set, fcntl() returns immediately with
		 * EAGAIN.
		 *
		 * F_SETLKW:
		 * This command is the same as F_SETLK except that if a shared or
		 * exclusive lock is blocked by other locks, the process waits
		 * until the request can be satisfied.  If a signal that is to be
		 * caught is received while fcntl() is waiting for a region, the
		 * fcntl() will be interrupted if the signal handler has not
		 * specified the SA_RESTART (see sigaction(2)).
		 */
		cmd = (cmdch == 'g') ? F_GETLK : (cmdch == 's') ? F_SETLK : F_SETLKW;

		fl.l_start = st;
		fl.l_len = len;
		/*
		 * F_RDLCK: shared or read lock.
		 * F_WRLCK: exclusive or write lock.
		 * F_UNLCK: unlock.
		 */
		fl.l_type = (lock == 'r') ? F_RDLCK : (lock == 'w') ? F_WRLCK : F_UNLCK;
		fl.l_whence = (whence == 's') ? SEEK_SET : (whence == 'c')
			? SEEK_CUR : SEEK_END;

		fd = fdset[filenum];
		/*
		 * NOTE: if you open a file with Read Only mode, the fcntl() system
		 * call will fail.
		 *
         * The argument cmd is F_SETLK or F_SETLKW, the type of lock (l_type)
		 * is a shared lock (F_RDLCK), and fd is not a valid file descriptor
		 * open for reading.
		 *
		 * The argument cmd is F_SETLK or F_SETLKW, the type of lock (l_type)
		 * is an exclusive lock (F_WRLCK), and fd is not a valid file
		 * descriptor open for writing.
		 */
		if ((status = fcntl(fd, cmd, &fl)) == -1)	/* Perform request... */
			errmsg_exit1("fcntl failed, %s\n", ERR_MSG);

		if (cmd == F_GETLK) {
			if (fl.l_type == F_UNLCK)
				printf("[PID=%d] Lock can be placed\n", getpid());
			else
				printf("[PID=%d] Denied by %s lock on %ld:%ld "
					"(hold by PID %d)\n", getpid(),
					(fl.l_type == F_RDLCK) ? "READ" : "WRITE",
					fl.l_start, fl.l_len, fl.l_pid);
		} else {
			if (status == 0) {
				printf("[PID=%d] %s\n", getpid(), (lock == 'u')
					? "unlocked" : "got lock");
			} else if (errno == EAGAIN || errno == EACCES)
                printf("[PID=%d] failed (incompatible lock)\n", getpid());
            else if (errno == EDEADLK)	/* F_SETLKW */
                printf("[PID=%ld] failed (deadlock)\n", (long) getpid());
            else
				errmsg_exit1("fcntl failed, %s\n", ERR_MSG);
		}
	}

	xfree(fdset);

	exit(EXIT_SUCCESS);
}

/* 
 * Mandatory locking is enabled for a file if the set-group-ID bit is on
 * but group-execute permission is off (a combination that under earlier
 * versions of UNIX had no useful meaning). If mandatory locking is enabled
 * for a file, then attempts to perform write(2) or read(2) calls on locked
 * regions of files will block until the lock is removed (or if the I/O
 * call is nonblocking it will return immediately with an error).
 */
static int
mdlock_enabled(int fd)
{
	struct stat st;

	if (fstat(fd, &st) == -1)
		errmsg_exit1("fstat failed, %s\n", ERR_MSG);

	return (st.st_mode & S_ISGID) != 0 && (st.st_mode & S_IXGRP) == 0;
}

static void 
display(int argc, char *argv[], const int *fdset)
{
	int i;

    if (argc == 2)	/* Only a single filename argument */
        printf("\nFormat: cmd lock start length [whence]\n\n");
    else {
        printf("\nFormat: %scmd lock start length [whence]\n\n",
			(argc > 2) ? "file-num " : "");
        printf("\tfile-num is a number from the following list\n");
        for (i = 1; i < argc; i++) {
            printf("\t\t%2d  %-10s [%s locking]\n", i, argv[i],
                mdlock_enabled(fdset[i]) ? "mandatory" : "advisory");
		}
    }
    printf("\t'cmd' is 'g' (GETLK), 's' (SETLK), or 'w' (SETLKW)\n");
    printf("\t'lock' is 'r' (READ), 'w' (WRITE), or 'u' (UNLOCK)\n");
    printf("\t'start' and 'length' specify byte range to lock\n");
    printf("\t'whence' is 's' (SEEK_SET, default), 'c' (SEEK_CUR), "
		"or 'e' (SEEK_END)\n");
	printf("\t'quit' or 'exit' to quit this program.\n\n");
}
