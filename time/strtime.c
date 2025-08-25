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
#include <time.h>
#include <locale.h>

int
main(int argc, char *argv[])
{
	struct tm tim;
	char buf[BUF_SIZE], *fmt;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s in-datetime in-format [out-format]\n",
			argv[0]);

	/*
	 * The setlocale() function sets the C library's notion of natural
	 * language formatting style for particular sets of routines. Each such
	 * style is called a ‘locale’ and is invoked using an appropriate name
	 * passed as a C string.
	 *
	 * The setlocale() function recognizes several categories of routines.
	 * These are the categories and the sets of routines they select:
	 *
	 * LC_ALL	Set the entire locale generically.
	 *
	 * Other categories see setlocale(3)
	 *
	 * Only three locales are defined by default, the empty string "" which
	 * denotes the native environment, and the "C" and "POSIX" locales,
	 * which denote the C language environment. A locale argument of NULL
	 * causes setlocale() to return the current locale.
	 */
	if (setlocale(LC_ALL, "") == NULL)
		errmsg_exit1("setlocale error\n");

	/*
	 * The strptime() function parses the string in the buffer buf according
	 * to the string pointed to by format, and fills in the elements of the
	 * structure pointed to by timeptr. The resulting values will be
	 * relative to the local time zone. Thus, it can be considered the
	 * reverse operation of strftime(3).
	 *
	 * The format string consists of zero or more conversion specifications
	 * and ordinary characters. All ordinary characters are matched exactly
	 * with the buffer, where white space in the format string will match
	 * any amount of white space in the buffer. All conversion
	 * specifications are identical to those described in strftime(3).
	 */
	memset(&tim, 0, sizeof(tim));
	if (strptime(argv[1], argv[2], &tim) == NULL)
		errmsg_exit1("strptime error\n");

	tim.tm_isdst = -1;	/* Not set by strptime(); tells mktime() to
				determine if DST is in effect */

	printf("calendar time (seconds since Epoch): %ld\n", mktime(&tim));

	/*
	 * The strftime() function formats the information from timeptr into
	 * the buffer buf according to the string pointed to by format.
	 *
	 * About conversion format see strftime(3)
	 */
	fmt = (argc > 3) ? argv[2] : "%F %T";
	if (strftime(buf, BUF_SIZE, fmt, &tim) == 0)
		errmsg_exit1("strftime error\n");
	printf("strftime() yields: %s\n", buf);

	exit(EXIT_SUCCESS);
}
