#define _POSIX_SOURCE
#include "timer.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// a stand alone timer
void start_timer(unsigned int sec) {
  pid_t pid = fork();
  if (pid == 0) {
    sleep(sec);
    exit(0);
  }
}

// for parent to check timeout.
// the parent process can exit for either waitpid or
// a EINTR errono.
// This allows us to exit if child finished but parent is still
// sleeping.
void waitpid_or_EINTR(int *status) {
  pid_t pid = 0;
  while ((pid = waitpid(-1, status, 0)) == -1) {
    if (errno == EINTR) // keep waiting
      continue;
    else { // kill the process early.
      if (pid != 0) {
        wait(NULL);
        printf("to kill %d : ", pid);
        kill(pid, SIGKILL);
      }
    }
  }
}
