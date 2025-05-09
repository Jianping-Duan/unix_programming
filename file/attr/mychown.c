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
#include <pwd.h>	/* getpwnam() */
#include <grp.h>	/* getgrnam() */

int
main(int argc, char *argv[])
{
	struct passwd *pwd;
	struct group *grp;
	uid_t uid;
	gid_t gid;
	int i, errfnd = 0;

	if (argc < 3)
		errmsg_exit1("Usages: %s owner group [file...]\n"
			"\towner or group can be '-', meaning leave unchanged\n", argv[0]);

	if (strcmp(argv[1], "-") == 0)
		uid = -1;
	else {
		if ((pwd = getpwnam(argv[1])) == NULL)
			errmsg_exit1("getpwnam failure, %s\n", ERR_MSG);
		uid = pwd->pw_uid;
	}

	if (strcmp(argv[2], "-") == 0)
		gid = -1;
	else {
		if ((grp = getgrnam(argv[2])) == NULL)
			errmsg_exit1("getgrnam failure, %s\n", ERR_MSG);
		gid = grp->gr_gid;
	}

	for (i = 3; i < argc; i++)
		if (chown(argv[i], uid, gid) == -1) {
			errmsg_exit1("chown %s failure, %s\n", argv[i], ERR_MSG);
			errfnd = 1;
		}

	exit(errfnd ? EXIT_FAILURE : EXIT_SUCCESS);
}
