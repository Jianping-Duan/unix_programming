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
#include <sys/time.h>

#define SECONDS_IN_YEAR	(365.24219 * 24 * 60 * 60)

int
main(void)
{
	time_t ts;
	struct timeval tv;
	struct tm gm, *gmp, loc, *locp;

	/* Retrieve time, convert and display it in various forms */

	/*
	 * The time() function returns the value of time in seconds since 0
	 * hours, 0 minutes, 0 seconds, January 1, 1970, Coordinated Universal
	 * Time (UTC). If an error occurs, time() returns the value (time_t)-1.
	 */
	if ((ts = time(NULL)) == -1)
		errmsg_exit1("time error\n");
	printf("Seconds since the Epoch (1 Jan 1970): %ld\n", ts);
	printf("\t(about %6.3f years)\n", (double)ts / SECONDS_IN_YEAR);

	/*
	 * The system's notion of the current Greenwich time and the current
	 * time zone is obtained with the gettimeofday() system call.
	 *
	 * The time is expressed in seconds and microseconds since midnight
	 * (0 hour), January 1, 1970. The resolution of the system clock is
	 * hardware dependent, and the time may be updated continuously or in
	 * “ticks”. If tp or tzp is NULL, the associated time information will
	 * not be returned or set
	 */
	if (gettimeofday(&tv, NULL) == -1)
		errmsg_exit1("gettimeofday failed, %s\n", ERR_MSG);
	printf("\tgettimeofday() returned %ld secs, %ld  microsecs\n",
		tv.tv_sec, tv.tv_usec);
	printf("\n");

	/*
	 * The gmtime() function take as argument a pointer to a time value
	 * representing the time in seconds since the Epoch
	 * (00:00:00 UTC on January 1, 1970; see time(3))
	 *
	 * The gmtime() function similarly converts the time value, but without
	 * any time zone adjustment, and returns a pointer to a struct tm.
	 *
	 * tm_sec	seconds (0 - 60)
	 * tm_min	minutes (0 - 59)
	 * tm_hour	hours (0 - 23)
	 * tm_mday	day of month (1 - 31)
	 * tm_mon	month of year (0 - 11)
	 * tm_year	year - 1900
	 * tm_wday	day of week (Sunday = 0)
	 * tm_yday	day of year (0 - 365)
	 * tm_isdst	is summer time in effect?
	 * tm_zone	abbreviation of timezone name
	 * tm_gmtoff	offset from UTC in seconds
	 */
	if ((gmp = gmtime(&ts)) == NULL)
		errmsg_exit1("gmtime error\n");

	gm = *gmp;	/* *gmp may be modified by asctime() or gmtime() */
	printf("Broken down by gmtime():\n");
	printf("\tyear=%d, mon=%d, day=%d, hour=%d, min=%d, sec=%d, ",
		gm.tm_year, gm.tm_mon, gm.tm_mday, gm.tm_hour, gm.tm_min,
		gm.tm_sec);
	printf("wday=%d, yday=%d, isdat=%d\n", gm.tm_wday, gm.tm_yday,
		gm.tm_isdst);
	printf("\n");

	/*
	 * The localtime() function converts the time value pointed to by clock,
	 * and returns a pointer to a struct tm (described below) which contains
	 * the broken-out time information for the value after adjusting for the
	 * current time zone (see tzset(3)). When the specified time translates
	 * to a year that will not fit in an int, localtime() returns NULL. The
	 * localtime() function uses tzset(3) to initialize time conversion
	 * information if tzset(3) has not already been called by the process.
	 *
	 * After filling in the struct tm, localtime() sets the tm_isdst'th
	 * element of tzname to a pointer to an ASCII string that is the time
	 * zone abbreviation to be used with localtime()'s return value.
	 */
	if ((locp = localtime(&ts)) == NULL)
		errmsg_exit1("localtime error\n");

	loc = *locp;
	printf("Broken down by localtime():\n");
	printf("\tyear=%d, mon=%d, day=%d, hour=%d, min=%d, sec=%d, ",
		loc.tm_year, loc.tm_mon, loc.tm_mday, loc.tm_hour, loc.tm_min,
		loc.tm_sec);
	printf("wday=%d, yday=%d, isdat=%d\n", loc.tm_wday, loc.tm_yday,
		loc.tm_isdst);
	printf("\n");

	/*
	 * The ctime() function adjusts the time value for the current time
	 * zone in the same manner as localtime(), and returns a pointer to a
	 * 26-character string of the form:
	 *
	 *	Thu Nov 24 18:22:48 1986\n\0
	 *
	 * All the fields have constant width.
	 *
	 * The asctime() function converts the broken down time in the struct tm
	 * pointed to by tm to the form shown in the example above
	 */
	printf("asctime() formats the gmtime() value as:\t%s", asctime(&gm));
	printf("ctime() formats the time() value as:\t\t%s", ctime(&ts));
	printf("\n");

	/*
	 * The functions mktime() convert the broken-down time in the struct tm
	 * pointed to by tm into a time value with the same encoding as that of
	 * the values returned by the time(3) function (that is, seconds from
	 * the Epoch, UTC). The mktime() function interprets the input structure
	 * according to the current timezone setting (see tzset(3))
	 */
	printf("mktime() of gmtime() value:\t\t%ld secs\n", mktime(&gm));
	printf("mktime() of localtime() value:\t\t%ld secs\n", mktime(&loc));

	exit(EXIT_SUCCESS);
}
