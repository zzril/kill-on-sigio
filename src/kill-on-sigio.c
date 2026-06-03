#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

// --------

static int g_fd_fifo = -1;
static pid_t g_pid_child = -1;

// --------

static void clean_exit(int status);
static int cleanup(void);
static int kill_child(int signum);
static void send_child_sigterm(int);
static int kill_child_report_errors(int signum);

// --------

static void clean_exit(int status) {
	cleanup();
	exit(status);
}

static int cleanup(void) {

	int status = 0;

	if(g_pid_child >= 0) {
		status |= kill_child_report_errors(SIGKILL);
	}

	if(g_fd_fifo >= 0) {
		if(close(g_fd_fifo) < 0) {
			perror("close");
			status |= -1;
		}
	}
	return status;
}

static int kill_child(int signum) {
	return kill(g_pid_child, signum);
}

static int kill_child_report_errors(int signum) {
	if(kill_child(signum) < 0) {
		perror("kill");
		fprintf(stderr, "could not kill: %d\n", g_pid_child);
		return -1;
	}
	return 0;
}

static void send_child_sigterm(__attribute__((unused)) int signum) {
	kill_child(SIGTERM);
}

// --------

int main(int argc, char** argv) {

	if(argc < 3) {
		fprintf(stderr, "usage: %s /path/to/fifo command\n", argv[0]);
		clean_exit(EXIT_FAILURE);
	}

	switch(g_pid_child=fork()) {

		case -1:
			perror("fork");
			clean_exit(EXIT_FAILURE);

		case 0:
			execvp(argv[2], argv + 2);
			perror("execve");
			clean_exit(EXIT_FAILURE);
	}

	struct sigaction act;
	memset((void*) &act, 0, sizeof(struct sigaction));
	act.sa_handler = &send_child_sigterm;

	if(sigaction(SIGIO, &act, NULL) < 0) {
		perror("sigaction");
		clean_exit(EXIT_FAILURE);
	}

	if((g_fd_fifo = open(argv[1], O_RDONLY | O_NONBLOCK)) < 0) {
		perror(argv[1]);
		clean_exit(EXIT_FAILURE);
	}

	if(fcntl(g_fd_fifo, F_SETOWN, getpid()) < 0) {
		perror("fcntl(F_SETOWN)");
		clean_exit(EXIT_FAILURE);
	}

	int oldflags = fcntl(g_fd_fifo, F_GETFL);
	if(oldflags < 0) { // if highest bit is used in the bitmask, the whole api is messed up
		perror("fcntl(F_GETFL)");
		clean_exit(EXIT_FAILURE);
	}

	if(fcntl(g_fd_fifo, F_SETFL, oldflags | O_ASYNC) < 0) {
		perror("fcntl(F_SETFL)");
		clean_exit(EXIT_FAILURE);
	}

	if(wait(NULL) >= 0 ) { // stopfile wasn't written to
		g_pid_child = -1;
		clean_exit(EXIT_SUCCESS);
	}

	if(errno != EINTR) {
		perror("wait");
		clean_exit(EXIT_FAILURE);
	}

	g_pid_child = -1; // child was sent the signal
	clean_exit(EXIT_SUCCESS);
}

