/*
 * test01 - create a blocker thread and a starving thread and see if
 * 		stalld fixes the issue
 */
#define _GNU_SOURCE
#include <ctype.h>
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
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <linux/sched.h>

/* behavior switches */
static long verbose = 0;
static long quiet = 0;
static long debugging = 0;

/* cpu core to use for test */
static int testcpu = -1;

/* FIFO priority for blocker */
static unsigned int blockerprio = 2;

/*
 * shared variable to indicate the
 * state of the two threads
 */
static unsigned int blocked = 1;

/* pthread barrier for synchronized start */
static pthread_barrier_t all_threads_ready;

/* thread routines */
static void *blockee(void *arg);
static void *blocker(void *arg);

static void process_command_line(int argc, char **argv);

static int allow_signal(int signum);
static void inthandler(int signo, siginfo_t *info, void *extra);
static void set_sig_handler();

/* thread ids */
static pthread_t blocker_tid;
static pthread_t blockee_tid;

#define BUFFERSIZE 1024

static void debug(const char *fmt, ...)
{
	va_list ap;

	if (debugging) {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}

static void error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (errno)
		fputs(strerror(errno), stderr);
}

static int isonline(int cpu)
{
	char buffer[BUFFERSIZE];
	FILE *fp;
	int online;

	sprintf(buffer, "/sys/devices/system/cpu/cpu%d/online", cpu);
	if (access(buffer, F_OK) == -1)
		return 1;

	fp= fopen(buffer, "r");
	if (fp == NULL)
		return 1;
	if (fscanf(fp, "%d", &online) != 1) {
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return online;
}

static int pick_cpu(void)
{
	int i;
	int ncpus = sysconf(_SC_NPROCESSORS_ONLN);

	for (i = ncpus-1; i > 0; i--) {
		if (isonline(i))
			return i;
	}
	return -1;
}

static int setup_thread(pthread_t *id, int cpu, int policy, int priority, void *(routine)(void *))
{
	int status;
	pthread_t tid;
	pthread_attr_t attr;
	cpu_set_t  cpuset;

	*id = 0;
	status = pthread_attr_init(&attr);
	if (status != 0) {
		error("failed to initialize pthread attribute struct");
		return status;
	}

	status = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (status != 0) {
		error("failed to set attr PTHREAD_EXPLICIT_SCHED\n");
		return status;
	}

	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	status = pthread_attr_setaffinity_np(&attr, sizeof(cpuset), &cpuset);
	if (status != 0) {
		error("failed to set blocker affinity to cpu %d: %s\n",
			cpu, strerror(errno));
		return status;
	}

	status = pthread_attr_setschedpolicy(&attr, policy);
	if (status != 0) {
		error("failed to set policy to %d: %s\n",
			policy, strerror(errno));
		return status;
	}

	if (priority > 0) {
		struct sched_param param;
		memset(&param, 0, sizeof(param));
		param.sched_priority = priority;
		status = pthread_attr_setschedparam(&attr, &param);
		if (status != 0) {
			error("failed to set priority to %d: %s\n",
				priority, strerror(errno));
			return status;
		}
	}

	status = pthread_create(&tid, &attr, routine, NULL);
	if (status != 0) {
		error("failed to create thread: %s\n", strerror(status));
		return status;
	}
	*id = tid;
	return 0;
}

static int setup_blocker(void)
{
	int status = setup_thread(&blocker_tid, testcpu, SCHED_FIFO, blockerprio, blocker);
	if (status) {
		error("failed to setup blocker thread\n");
		exit(status);
	}
	debug("blocker thread id: %ld\n", blocker_tid);
	return status;
}

static int setup_blockee(void)
{
	int status = setup_thread(&blockee_tid, testcpu, SCHED_OTHER, 0, blockee);
	if (status) {
		error("failed to create blockee thread\n");
		exit(status);
	}
	debug("blockee thread id: %ld\n", blockee_tid);
	return status;
}

static void usage(void)
{
	printf("usage: test01 [-c N] [-p N] [-v] [-d] [-q]\n");
}

struct option options[] = {
	{ "help", 	no_argument, 		NULL, 	'h' },
	{ "cpu", 	required_argument, 	NULL,	'c' },
	{ "priority", 	required_argument, 	NULL,	'p' },
	{ "verbose", 	no_argument, 		NULL, 	'v' },
	{ "quiet", 	no_argument, 		NULL, 	'q' },
	{ "debug", 	no_argument, 		NULL, 	'd' },
	{ 0, 		0, 			0, 	0 }
};

static void process_command_line(int argc, char **argv)
{
	int opt;
	while ((opt = getopt_long(argc, argv, "hvqp:c:d", options, NULL)) != -1) {
		switch (opt) {
		case 'h':
			usage();
			exit(0);
		case 'c':
			testcpu = atoi(optarg);
			break;
		case 'p':
			blockerprio = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			quiet = 0;
			break;
		case 'q':
			verbose = 0;
			quiet = 1;
			break;
		case 'd':
			debugging = 1;
			verbose = 1;
			break;
		}
	}
}

/*
 * loop decrementing a variable until it hits zero
 */
static void *blockee(void *arg)
{
	int ret;

	ret = pthread_barrier_wait(&all_threads_ready);
	debug("blockee: running\n");

	if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0) {
		error("barrier wait in blocker failed");
		return (void *) -1;
	}
	while(blocked > 0) {
		debug("blockee: executing loop body, blocked==%d\n",
		      blocked);
		blocked--;
	}
	debug("blockee: finished!\n");
	return 0;
}

/*
 * loop waiting for blocked variable to go to zero
 */

static void *blocker(void *arg)
{
	int ret = pthread_barrier_wait(&all_threads_ready);

	if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0) {
		error("barrier wait in blocker failed");
		return (void *) -1;
	}
	debug("blocker: running\n");

	while(blocked > 0)
		;

	debug("blocker: finished!\n");
	return 0;
}

int main (int argc, char **argv)
{
	int status;
	cpu_set_t cpuset;
	struct sched_param param;

	/* handle the command line options */
	process_command_line(argc, argv);

	/* setup to handle SIGINT */
	allow_signal(SIGINT);
	set_sig_handler();


	/* set up our ready barrier */
	status = pthread_barrier_init(&all_threads_ready, NULL, 3);
	if ((status ) != 0) {
		error("pthread_barrier_init");
		exit(errno);
	}

	/* if one wasn't specified, pick a core on which to test */
	if (testcpu == -1)
		testcpu = pick_cpu();

	debug("main:  testcpu: %d\n", testcpu);

	if (setup_blocker() != 0) {
		error("setting up blocker failed\n");
		exit(errno);
	}
	debug("main: blocker thread started (tid: %ld)\n", blocker_tid);

	if (setup_blockee() != 0) {
		error("setting up blockee failed\n");
		exit(errno);
	}
	debug("main: blockee thread started (tid: %ld)\n", blockee_tid);

	/*
	 * ensure that main doesn't run on the test cpu
	 */
	debug("set main affinity to not use cpu %d\n", testcpu);
	CPU_ZERO(&cpuset);
	status = sched_getaffinity(0, sizeof(cpuset), &cpuset);
	if (status < 0) {
		error("Error getting main affinity");
		exit(errno);
	}
	CPU_CLR(testcpu, &cpuset);
	status = sched_setaffinity(0, sizeof(cpuset), &cpuset);
	if (status < 0) {
		error("main: Error setting original affinity");
		exit(errno);
	}

	/*
	 * make our main thread run SCHED_FIFO priority one greater
	 * than the blocker thread (just for safety's sake)
	 */
	memset(&param, 0, sizeof(param));
	param.sched_priority = blockerprio+1;
	status = sched_setscheduler(0, SCHED_FIFO, &param);
	if (status < 0) {
		error("main: error setting scheduler policy to FIFO: %s",
		      strerror(errno));
		exit(errno);
	}

	/*
	 * start the blocker and blockee
	 */
	debug("main: calling pthread_barrier_wait to start threads\n");

	status = pthread_barrier_wait(&all_threads_ready);
	if (status != PTHREAD_BARRIER_SERIAL_THREAD && status != 0) {
		error("main error from pthread_barrier_wait");
		exit(errno);
	}

	debug("main: waiting for blocker to exit\n");
	status = pthread_join(blocker_tid, NULL);
	if (status < 0) {
		error("Error joining blocker thread");
		exit(errno);
	}
	debug("main: Joined blocker\n");

	debug("main: waiting for blockee to exit\n");
	status = pthread_join(blockee_tid, NULL);
	if (status < 0) {
		error("Error joining blockee thread");
		exit(errno);
	}
	debug("main: Joined blockee\n");

	printf("test completed successfully!\n");
	exit(0);
}

/*
 * SIGINT handler for main
 */
static void inthandler (int signo, siginfo_t *info, void *extra)
{
	debug("got SIGINT\n");
	if (blocker_tid) {
		debug("sending SIGTERM to blocker\n");
		pthread_kill(blocker_tid, SIGTERM);
	}
	if (blockee_tid) {
		debug("sending SIGTERM to blockee\n");
		pthread_kill(blockee_tid, SIGTERM);
	}
	debug("exiting due to SIGINT\n");
	exit(-1);
}

static void set_sig_handler()
{
	struct sigaction action;
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = inthandler;
	if (sigaction(SIGINT, &action, NULL) == -1) {
		error("error setting SIGINT handler: %s\n",
		      strerror(errno));
		exit(errno);
	}
}

/*
 * block all signals except the specified input signal
 */
static int allow_signal(int signum)
{
	int status;
	sigset_t sigset;

	/* mask off all signals */
	status = sigfillset(&sigset);
	if (status) {
		error("setting up full signal set %s\n", strerror(status));
		return status;
	}
	status = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	if (status) {
		error("setting signal mask: %s\n", strerror(status));
		return status;
	}

	/* now allow signum to be delivered */
	status = sigemptyset(&sigset);
	if (status) {
		error("creating empty signal set: %s\n", strerror(status));
		return status;
	}
	status = sigaddset(&sigset, signum);
	if (status) {
		error("adding %d to signal set: %s\n", signum, strerror(status));
		return status;
	}
	status = pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
	if (status) {
		error("unblocking signal %d: %s\n", signum, strerror(status));
		return status;
	}
	return 0;
}
