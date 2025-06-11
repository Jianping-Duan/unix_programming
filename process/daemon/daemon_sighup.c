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
#include <sys/stat.h>	/* umask() */
#include <fcntl.h>
#include <signal.h>
#include <time.h>

static const char *LOG_FILE = "/tmp/daemon_sighup.log";
static const char *CONFIG_FILE = "/tmp/daemon_sighup.conf";

static FILE *logfp;
static volatile sig_atomic_t hup_received = 0;

#define	BD_NO_CHDIR				01	/* Don't chdir('/') */
#define BD_NO_CLOSE_FILES		02	/* Don't close all open files */
#define BD_NO_REOPEN_STDFDS		04	/* Don't reopen stdin, stdout and stderr
									   to '/dev/null' */
#define BD_NO_UMASK0			010	/* Don't do a umask(0) */
#define BD_MAX_CLOSE	8192	/* Maximum file descriptors to close if
								   sysconf(_SC_OPEN_MAX) is indeterminate */
static int become_daemon(int);

static void log_open(const char *);
static void log_message(const char *, ...);
static void log_close(void);
static void read_config_file(const char *);

static void sighup_handler(int);

int
main(void)
{
#define SLEEP_TIME	15
	int errcode, cnt = 0, unslept;
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sighup_handler;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
		errmsg_exit1("sigaction - SIGHUP failed, %s\n", ERR_MSG);

	if ((errcode = become_daemon(0)) != 0)
		errmsg_exit1("become_daemon failed, errcode = %d\n", errcode);

	log_open(LOG_FILE);
	read_config_file(CONFIG_FILE);

	unslept = SLEEP_TIME;
	while (1) {
		/*
		 * If the sleep() function returns because the requested time has
		 * elapsed, the value returned will be zero. If the sleep() function
		 * returns due to the delivery of a signal, the value returned will be
		 * the unslept amount (the requested time minus the time actually slept)
		 * in seconds.
		 */
		unslept = sleep(unslept);	/*  Returns > 0 if interrupted */

		if (hup_received) {		/* If we got SIGHUP... */
			hup_received = 0;	/* Get ready for next SIGHUP */
			log_close();
			log_open(LOG_FILE);
			read_config_file(CONFIG_FILE);
		}

		if (unslept == 0) {		/* On completed interval */
			cnt++;
			log_message("Main: %d", cnt);
			unslept = SLEEP_TIME;	/* Reset interval */
		}
	}
}

static int
become_daemon(int flags)
{
	int maxfd, fd;

	/* Become background process */
	switch(fork()) {
		case -1: return -1;
		case 0: break;					/* Child falls through...*/
		default: _exit(EXIT_SUCCESS);	/* while parent terminates */
	}

	/* Become leader of new session */
	if (setsid() == -1)
		return -2;

	/* Ensure we are not session leader */
	switch(fork()) {
		case -1: return -1;
		case 0: break;					/* Child falls through...*/
		default: _exit(EXIT_SUCCESS);	/* while parent terminates */
	}

	if (!(flags & BD_NO_UMASK0))
		umask(0);	/* Clear file mode creation mask */

	if (!(flags & BD_NO_CHDIR))
		chdir("/");	/* Change to root directory */

	if (!(flags & BD_NO_CLOSE_FILES)) {	/* Close all open files */
		/* Limit is indeterminate... */
		if ((maxfd = sysconf(_SC_OPEN_MAX)) == -1)
			maxfd = BD_MAX_CLOSE;	/* take a guess */
		for (fd = 0; fd < maxfd; fd++)
			close(fd);
	}

	if (!(flags & BD_NO_REOPEN_STDFDS)) {
		close(STDIN_FILENO);	/* Reopen standard fd's to /dev/null */
		
		/* 'fd' should be 0 */
		if ((fd = open("/dev/null", O_RDWR)) != STDIN_FILENO)
			return -3;

		if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
			return -4;
		if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
			return -5;
	}

	return 0;
}

static void
sighup_handler(int sig)
{
	hup_received = 1;
	assert(sig == SIGHUP);
}

static void
log_open(const char *filename)
{
	mode_t m;

	m = umask(077);
	logfp = fopen(filename, "a");
	umask(m);

	if (logfp == NULL)
		exit(EXIT_FAILURE);
	
	setvbuf(logfp, NULL, _IONBF, 0);
	log_message("Opened log file.");
}

static void
log_message(const char *format, ...)
{
#define TSFMT	"%F %X"		/* YYYY-MM-DD HH:MM:SS */
#define TS_BUFSZ	(sizeof (" YYYY-MM-DD HH:MM:SS"))
	va_list ap;
	char ts[TS_BUFSZ];
	time_t vtim;
	struct tm *loc;

	vtim = time(NULL);
	loc = localtime(&vtim);
	if (loc == NULL || strftime(ts, TS_BUFSZ, TSFMT, loc) == 0)
		fprintf(logfp, "??? Unknow time ???: ");
	else
		fprintf(logfp, "%s: ", ts);

	va_start(ap, format);
	vfprintf(logfp, format, ap);
	fprintf(logfp, "\n");
	va_end(ap);
}

static void
log_close(void)
{
	log_message("Closing log file.");
	fclose(logfp);
}

static void
read_config_file(const char *filename)
{
	FILE *fp;
	char line[BUF_SIZE];

	if ((fp = fopen(filename, "r")) == NULL)
		return;
	if (fgets(line, BUF_SIZE, fp) == NULL)
		line[0] = '\0';
	else
		line[strlen(line) - 1] = '\0';	/* Strip trailing '\n' */
	
	log_message("Read config file: %s", line);
	fclose(fp);
}
