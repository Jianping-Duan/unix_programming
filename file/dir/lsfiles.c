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
#include <dirent.h>

static void lsfiles(const char *);

int
main(int argc, char *argv[])
{
	if (argc < 1)
		errmsg_exit1("Usage: %s [dir-path...]\n", argv[0]);

	if (argc == 1)
		lsfiles(".");
	else
		for (++argv; *argv; argv++)
			lsfiles(*argv);

	exit(EXIT_SUCCESS);
}

static void
lsfiles(const char *dirpath)
{
	DIR *dirp;
	struct dirent *dp;
	bool iscurdir = false;

	iscurdir = strcmp(dirpath, ".") == 0;	/* true if 'dirpath' is "." */

	if ((dirp = opendir(dirpath)) == NULL)
		errmsg_exit1("opendir failed on '%s', %s\n", dirpath, ERR_MSG);

	/* 
	 * For each entry in this directory,
	 * print directory + filename.
	 */
	while(1) {
		errno = 0;	/* To distinguish error from end-of-directory */
		if ((dp = readdir(dirp)) == NULL)
			break;

		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
			continue;	/* skip "." and ".." */

		if (!iscurdir)
			printf("%s/", dirpath);
		printf("%s\n", dp->d_name);
	}

	if (errno != 0)
		errmsg_exit1("readdir failed, %s\n", ERR_MSG);

	if (closedir(dirp) == -1)
		errmsg_exit1("closedir error, %s\n", ERR_MSG);
}
