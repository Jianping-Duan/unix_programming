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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <getopt.h>

static void usage_info(const char *, const char *);

int
main(int argc, char *argv[])
{
	const char *optstr = "cf:k:px";
	int opt, flags = 0, kcnt = 0;
	int semid, nsems;
	mode_t perms;
	key_t key;
	long ukey;

	extern char *optarg;
	extern int optind;

	while ((opt = getopt(argc, argv, optstr)) != -1) {
		switch (opt) {
		case 'c':
			flags |= IPC_CREAT;
			break;
		case 'f':
			if ((key = ftok(optarg, 1)) == -1)
				errmsg_exit1("ftok failed\n");
			kcnt++;
			break;
		case 'k':
			if (sscanf(optarg, "%ld", &ukey) != 1)
				errmsg_exit1("-k option requires a numeric "
					"argument, %s\n", optarg);
			key = ukey;
			kcnt++;
			break;
		case 'p':
			key = IPC_PRIVATE;
			kcnt++;
			break;
		case 'x':
			flags |= IPC_EXCL;
			break;
		default:
			usage_info(argv[0], "Bad option");
		}
	}

	if (kcnt != 1)
		usage_info(argv[0], "Exactly one of the options -f, -k, or -p "
			"must be supplied\n");

	if (optind >= argc)
		usage_info(argv[0], "Must specify number of semaphores\n");

	nsems = getint(argv[optind]);
	perms = (optind + 1 >= argc) ? (S_IRUSR | S_IWUSR) :
		getlong(argv[optind + 1], GN_BASE_8);
	flags |= perms;

	if ((semid = semget(key, nsems, flags)) == -1)
		errmsg_exit1("semget failed, %s\n", ERR_MSG);

	printf("semid = %d\n", semid);

	exit(EXIT_SUCCESS);
}

static void
usage_info(const char *pname, const char *msg)
{
	if (msg != NULL)
		fprintf(stderr, "%s\n", msg);

	fprintf(stderr, "Usage: %s [-cx] {-f pathname | -k key | -p} "
		"num-sems [octal-perms]\n", pname);
	fprintf(stderr, "\t-c\t\tUse IPC_CREAT flag\n");
	fprintf(stderr, "\t-x\t\tUse IPC_EXCl flag\n");
	fprintf(stderr, "\t-f pathname Generate key using ftok\n");
	fprintf(stderr, "\t-k key\tUse 'key' as key\n");
	fprintf(stderr, "\t-p\t\tUse IPC_PRIVATE key\n");
	exit(EXIT_FAILURE);
}
