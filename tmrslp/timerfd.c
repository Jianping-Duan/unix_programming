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
#include <sys/timerfd.h>
#include <time.h>
#include "strtoitrspec.h"

int
main(int argc, char *argv[])
{
	int fd;
	long maxexp, totexp, numexp, secs, nsecs;
	struct itimerspec ts;
	struct timespec start, end;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s secs[/nsecs][:int-secs[/int-nsecs]] "
			"[max-exp]\n", argv[0]);

	strtoitrspec(argv[1], &ts);
	maxexp = (argc > 2) ? getlong(argv[2], 0) : 1;

	/*
	 * The timerfd system calls operate on timers, identified by special
	 * timerfd file descriptors.
	 *
	 * All timerfd descriptors possess traditional file descriptor
	 * semantics; they may be passed to other processes, preserved across
	 * fork(2), and monitored via kevent(2), poll(2), or select(2). When a
	 * timerfd descriptor is no longer needed, it may be disposed of using
	 * close(2).
	 *
	 * The timerfd_create(), initialize a timerfd object and return its file
	 * descriptor. The clockid argument specifies the clock used as a timing
	 * base and may be:
	 *
	 * CLOCK_REALTIME	Increments as a wall clock should.
	 * CLOCK_MONOTONIC	Increments monotonically in SI seconds.
	 *
	 * The flags argument may contain the result of or'ing he following
	 * values:
	 *
	 * TFD_CLOEXEC		The newly generated file descriptor will
	 *			close-on-exec.
	 * TFD_NONBLOCK		Do not block on read/write operations.
	 */
	if ((fd = timerfd_create(CLOCK_REALTIME, 0)) == -1)
		errmsg_exit1("timerfd_create failed, %s\n", ERR_MSG);

	/*
	 * Update the timer denoted by fd with the struct itimerspec in
	 * new_value. The it_value member of new_value should contain the amount
	 * of time before the timer expires, or zero if the timer should be
	 * disarmed. The it_interval member should contain the reload time if an
	 * interval timer is desired.
	 *
	 * The previous timer state will be stored in old_value given old_value
	 * is not NULL.
	 *
	 * The flags argument may contain the result of or'ing the following
	 * values:
	 *
	 * TFD_TIMER_ABSTIME	Expiration will occur at the absolute time
	 *			provided in new_value. Normally, new_value
	 *			represents a relative time compared to the
	 *			timer's clockid clock.
	 * TFD_TIMER_CANCEL_ON_SET
	 *	If clockid has been set to CLOCK_REALTIME and the realtime clock
	 *	has experienced a discontinuous jump, then the timer will be
	 *	canceled and the next read(2) will fail with ECANCELED.
	 */
	if (timerfd_settime(fd, 0, &ts, NULL) == -1)
		errmsg_exit1("timerfd_settime failed, %s\n", ERR_MSG);

	/*
	 * POSIX clock.
	 *
	 * The clock_gettime() and clock_settime() system calls allow the
	 * calling process to retrieve or set the value used by a clock which is
	 * specified by clock_id.
	 *
	 * CLOCK_MONOTONIC	Increments in SI seconds.
	 */
	if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
		errmsg_exit1("clock_gettime failed, %s\n", ERR_MSG);

	for (totexp = 0; totexp < maxexp;) {
		if (read(fd, &numexp, sizeof(numexp)) != sizeof(numexp))
			errmsg_exit1("read failed, %s\n", ERR_MSG);
		totexp += numexp;

		if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			errmsg_exit1("clock_gettime failed, %s\n", ERR_MSG);

		secs = end.tv_sec - start.tv_sec;
		if ((nsecs = end.tv_nsec - start.tv_nsec) < 0) {
			secs--;
			nsecs += 1000000000;
		}

		printf("%ld.%03ld: expirations read: %ld; total=%ld\n", secs,
			(nsecs + 500000) / 1000000, numexp, totexp);
	}

	if (close(fd) == -1)
		errmsg_exit1("close failed, %s\n", ERR_MSG);

	exit(EXIT_SUCCESS);
}
