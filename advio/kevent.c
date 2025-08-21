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
#include <sys/event.h>
#include <fcntl.h>
#include <sys/stat.h>

int
main(int argc, char *argv[])
{
	int nofs, kqfd, fd, i, j, ready, fflags;
	unsigned long ident;
	struct kevent *chglst, *evelst;
	char buf[BUF_SIZE];
	ssize_t nr;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s file...\n", argv[0]);

	/*
	 * The kqueue() system call provides a generic method of notifying the
	 * user when an event happens or a condition holds, based on the results
	 * of small pieces of kernel code termed filters. A kevent is identified
	 * by the (ident, filter) pair; there may only be one unique kevent per
	 * kqueue.
	 *
	 * The kqueue() system call creates a new kernel event queue and returns
	 * a descriptor. The queue is not inherited by a child created with
	 * fork(2). However, if rfork(2) is called without the RFFDG flag, then
	 * the descriptor table is shared, which will allow sharing of the
	 * kqueue between two processes.
	 */
	if ((kqfd = kqueue()) == -1)
		errmsg_exit1("kqueue failed, %s\n", ERR_MSG);

	nofs = argc - 1;
	chglst = xcalloc(nofs, sizeof(*chglst));
	evelst = xcalloc(nofs, sizeof(*evelst));

	/*
	 * EV_SET(kev, ident, filter, flags, fflags, data, udata);
	 *
	 * EV_ADD	Adds the event to the kqueue. Re-adding an existing
	 *		event will modify the parameters of the original event,
	 *		and not result in a duplicate entry. Adding an event
	 *		automatically enables it, unless overridden by the
	 *		EV_DISABLE flag.
	 *
	 * EV_ENABLE	Permit kevent() to return the event if it is triggered.
	 *
	 * EV_ONESHOT   Causes the event to return only the first occurrence of
	 *		the filter being triggered. After the user retrieves
	 *		the event from the kqueue, it is deleted.
	 *
	 * EVFILT_VNODE:
	 *	Takes a file descriptor as the identifier and the events to
	 *	watch for in fflags, and returns when one or more of the
	 *	requested events occurs on the descriptor.
	 *	The events to monitor are
	 *
	 *	NOTE_ATTRIB	The file referenced by the descriptor had its
	 *			attributes changed.
	 *
	 *	NOTE_DELETE	The unlink() system call was called on the file
	 *			referenced by the descriptor.
	 *
	 *	NOTE_EXTEND	For regular file, the file referenced by the
	 *			descriptor was extended.
	 *
	 *	NOTE_WRITE	A write occurred on the file referenced by the
	 *			descriptor
	 */
	for (i = 1, j = 0; i < argc; i++, j++) {
		if ((fd = open(argv[i], O_RDONLY)) == -1)
			errmsg_exit1("open %s failed, %s\n", argv[i], ERR_MSG);
		printf("Opened '%s' on fd %d\n", argv[i], fd);

		EV_SET(&chglst[j], fd, EVFILT_VNODE,
			EV_ADD | EV_ENABLE | EV_ONESHOT,
			NOTE_WRITE | NOTE_ATTRIB | NOTE_DELETE | NOTE_EXTEND,
			0, NULL);
	}

	/*
	 * The kevent() system call is used to register events with the queue,
	 * and return any pending events to the user. The changelist argument is
	 * a pointer to an array of kevent structures, as defined in
	 * <sys/event.h>. All changes contained in the changelist are applied
	 * before any pending events are read from the queue. The nchanges
	 * argument gives the size of changelist. The eventlist argument is a
	 * pointer to an array of kevent structures. The nevents argument
	 * determines the size of eventlist. When nevents is zero, kevent() will
	 * return immediately even if there is a timeout specified unlike
	 * select(2). If timeout is a non-NULL pointer, it specifies a maximum
	 * interval to wait for an event, which will be interpreted as a struct
	 * timespec. If timeout is a NULL pointer, kevent() waits indefinitely.
	 * To effect a poll, the timeout argument should be non-NULL, pointing
	 * to a zero-valued timespec structure. The same array may be used for
	 * the changelist and eventlist.
	 *
	 * The kevent() system call returns the number of events placed in the
	 * eventlist, up to the value given by nevents. If an error occurs while
	 * processing an element of the changelist and there is enough room in
	 * the eventlist, then the event will be placed in the eventlist with
	 * EV_ERROR set in flags and the system error in data. Otherwise, -1
	 * will be returned, and errno will be set to indicate the error
	 * condition. If the time limit expires, then kevent() returns 0
	 */
	while (nofs > 0) {
		ready = kevent(kqfd, chglst, nofs, evelst, nofs, NULL);
		if (ready == -1)
			errmsg_exit1("kevent (filter) failed, %s\n", ERR_MSG);
		printf("Ready: %d\n", ready);

		for (i = 0; i < ready; i++) {
			ident = evelst[i].ident;
			fflags = evelst[i].fflags;

			if (evelst[i].flags & EV_EOF) {
				fprintf(stderr, "%lu: EOF detected\n", ident);
				printf("\tclosing fd %lu\n", ident);
				if (close(ident) == -1)
					fprintf(stderr, "close failed, %lu\n",
						ident);
				nofs--;
				continue;
			}

			if (evelst[i].flags & EV_ERROR) {
				fprintf(stderr, "EV_ERROR, %s\n",
					strerror(evelst[j].data));
				printf("\tclosing fd %lu\n", ident);
				if (close(ident) == -1)
					fprintf(stderr, "close failed, %lu\n",
						ident);
				nofs--;
				continue;
			}

			if (fflags & NOTE_WRITE) {
				memset(buf, 0, BUF_SIZE);
				if ((nr = read(ident, buf, BUF_SIZE)) == -1)
					errmsg_exit1("read failed\n");
				printf("\tRead %ld bytes: %s\n", nr, buf);
			} else if (fflags & NOTE_DELETE) {
				printf("\t%lu: deleted\n", ident);
			} else if (fflags & NOTE_EXTEND) {
				printf("\t%lu: modified\n", ident);
			} else if (fflags & NOTE_ATTRIB) {
				printf("\t%lu: attributes modified\n", ident);
			} else {
				printf("\t others filter flag, 0x%x\n", fflags);
			}
		}
	}

	xfree(chglst);
	xfree(evelst);
	close(kqfd);

	printf("All file descriptors closed\n");
	exit(EXIT_SUCCESS);
}
