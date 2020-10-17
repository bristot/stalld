/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2020 Red Hat Inc, Clark Williams <williams@redhat.com>
 *
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <linux/sched.h>
#include <sys/sysinfo.h>

#include "stalld.h"

long get_long_from_str(char *start)
{
	long value;
	char *end;

	errno = 0;
	value = strtol(start, &end, 10);
	if (errno || start == end) {
		warn("Invalid ID '%s'", value);
		return -1;
	}

	return value;
}

long get_long_after_colon(char *start)
{
	/*
	 * Find the ":"
	 */
	start = strstr(start, ":");
	if (!start)
		return -1;

	/*
	 * skip ":"
	 */
	start++;

	return get_long_from_str(start);
}

long get_variable_long_value(char *buffer, const char *variable)
{
	char *start;
	/*
	 * Line:
	 * '  .nr_running                    : 0'
	 */

	/*
	 * Find the ".nr_running"
	 */
	start = strstr(buffer, variable);
	if (!start)
		return -1;

	return get_long_after_colon(start);
}

/*
 * SIGINT handler for main
 */
static void inthandler (int signo, siginfo_t *info, void *extra)
{
	log_msg("received signal %d, starting shutdown\n", signo);
	running = 0;
}

static void set_sig_handler()
{
	struct sigaction action;
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = inthandler;
	if (sigaction(SIGINT, &action, NULL) == -1) {
		warn("error setting SIGINT handler: %s\n",
		      strerror(errno));
		exit(errno);
	}
}

int setup_signal_handling(void)
{
	int status;
	sigset_t sigset;

	/* mask off all signals */
	status = sigfillset(&sigset);
	if (status) {
		warn("setting up full signal set %s\n", strerror(status));
		return status;
	}
	status = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	if (status) {
		warn("setting signal mask: %s\n", strerror(status));
		return status;
	}

	/* now allow SIGINT and SIGTERM to be delivered */
	status = sigemptyset(&sigset);
	if (status) {
		warn("creating empty signal set: %s\n", strerror(status));
		return status;
	}
	status = sigaddset(&sigset, SIGINT);
	if (status) {
		warn("adding SIGINT to signal set: %s\n", strerror(status));
		return status;
	}
	status = sigaddset(&sigset, SIGTERM);
	if (status) {
		warn("adding SIGTERM to signal set: %s\n", strerror(status));
		return status;
	}
	status = pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
	if (status) {
		warn("unblocking signals: %s\n", strerror(status));
		return status;
	}

	/* now register our signal handler */
	set_sig_handler();
	return 0;
}
