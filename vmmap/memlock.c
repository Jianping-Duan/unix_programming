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
#include <sys/mman.h>

static void display_mincore(const void *, size_t);

int
main(int argc, char *argv[])
{
	void *addr;
	size_t len, lcklen;
	long pagesz, stepsz, i;

	if (argc != 4)
		errmsg_exit1("Usage: %s num-pages lock-page-step "
			"lock-page-len\n", argv[0]);

	/*
	 * IEEE Std 1003.1-2001 (“POSIX.1”) removed getpagesize. Portable
	 * applications should use ‘sysconf(_SC_PAGESIZE)’ instead.
	 */
	if ((pagesz = sysconf(_SC_PAGESIZE)) == -1)
		errmsg_exit1("sysconf(_SC_PAGESIZE) failed, %s\n", ERR_MSG);

	assert((len = getint(argv[1]) * pagesz) > 0);
	assert((stepsz = getint(argv[2]) * pagesz) > 0);
	assert((lcklen = getint(argv[3]) * pagesz) > 0);

	addr = mmap(NULL, len, PROT_READ, MAP_SHARED | MAP_ANON, -1, 0);
	if (addr == MAP_FAILED)
		errmsg_exit1("mmap failed, %s\n", ERR_MSG);

	printf("Allocated %lu (%#lx) bytes started at %p\n", len, len, addr);

	printf("Before mlock():\n");
	display_mincore(addr, len);

	/*
	 * The mlock() system call locks into memory the physical pages
	 * associated with the virtual address range starting at addr for len
	 * bytes. 
	 *
	 * The addr argument should be aligned to a multiple of the page size.
	 * If the len argument is not a multiple of the page size, it will be
	 * rounded up to be so. The entire range must be allocated.
	 *
	 * After an mlock() system call, the indicated pages will cause neither
	 * a non-resident page nor address-translation fault until they are
	 * unlocked. They may still cause protection-violation faults or
	 * TLB-miss faults on architectures with software-managed TLBs. The
	 * physical pages remain in memory until all locked mappings for the
	 * pages are removed. Multiple processes may have the same physical
	 * pages locked via their own virtual address mappings. A single process
	 * may likewise have pages multiply-locked via different virtual
	 * mappings of the same physical pages.
	 */
	for (i = 0; i + lcklen < len; i += stepsz)
		if (mlock(addr + i, lcklen) == -1)
			errmsg_exit1("mlock failed, %s\n", ERR_MSG);

	printf("After mlock():\n");
	display_mincore(addr, len);

	exit(EXIT_SUCCESS);
}

static void
display_mincore(const void *addr, size_t len)
{
	char *vec;
	long pagesz, npages, i;

	pagesz = sysconf(_SC_PAGESIZE);
	npages = (len + pagesz - 1) / pagesz;

	vec = xmalloc(npages);

	/*
	 * The mincore() system call determines whether each of the pages in the
	 * region beginning at addr and continuing for len bytes is resident or
	 * mapped, depending on the value of sysctl vm.mincore_mapped. The
	 * status is returned in the vec array, one character per page. Each
	 * character is either 0 if the page is not resident, or a combination
	 * of the following flags (defined in <sys/mman.h>):
	 *
	 * MINCORE_INCORE		Page is in core (resident).
	 *
	 * MINCORE_REFERENCED		Page has been referenced by us.
	 *
	 * MINCORE_MODIFIED		Page has been modified by us.
	 *
	 * MINCORE_REFERENCED_OTHER	Page has been referenced.
	 *
	 * MINCORE_MODIFIED_OTHER	Page has been modified.
	 *
	 * MINCORE_PSIND(i)		Page is part of a large (“super”) page
	 *				with size given by index i in the array
	 *				returned by getpagesizes(3).
	 *
	 * MINCORE_SUPER		A mask of the valid MINCORE_PSIND()
	 *				values. If any bits in this mask are
	 *				set, the page is part of a large
	 *				("super") page
	 */
	if (mincore(addr, len, vec) == -1)
		errmsg_exit1("mincore failed, %s\n", ERR_MSG);

	for (i = 0; i < npages; i++) {
		if (i % 64 == 0)
			printf("%s%10p: ", (i == 0) ? "" : "\n",
				addr + (i * pagesz));
		printf("%c", (vec[i] & MINCORE_INCORE) ? '*' : '.');
	}
	printf("\n");

	xfree(vec);
}
