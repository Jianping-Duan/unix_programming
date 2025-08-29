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
#ifndef _STRTOITRSPEC_H_
#define _STRTOITRSPEC_H_

/*
 * Convert a string of the following form to an itimerspec structure:
 * "value.sec[/value.nanosec][:interval.sec[/interval.nanosec]]".
 * Optional components that are omitted cause 0 to be assigned to the
 * corresponding structure fields.
 */
static void
strtoitrspec(const char *str, struct itimerspec *its)
{
	char *cptr, *sptr;

	if ((cptr = strchr(str, ':')) != NULL)
		*cptr = '\0';

	if ((sptr = strchr(str, '/')) != NULL)
		*sptr = '\0';

	its->it_value.tv_sec = atoi(str);	/* endpoint '/' */
	its->it_value.tv_nsec = (sptr != NULL) ? atoi(sptr + 1) : 0;

	if (cptr == NULL) {
		its->it_interval.tv_sec = 0;
		its->it_interval.tv_nsec = 0;
	} else {
		if ((sptr = strchr(cptr + 1, '/')) != NULL)
			*sptr = '\0';

		its->it_interval.tv_sec = atoi(cptr + 1);
		its->it_interval.tv_nsec = (sptr != NULL) ? atoi(sptr + 1) : 0;
	}
}

#endif	/* !_STRTOITRSPEC_H_ */
