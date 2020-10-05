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

typedef struct {
  char cmd[81];
  char *cmdTokens[20];
  int count;
  int parallel : 1;
  int timeout;
} CloshState;

/*
 * Read command line arguments from the input to pass to the executing process.
 */
void readTokens(CloshState *s) {
  s->cmd[strlen(s->cmd) - 1] = '\0';
  int i = 0;
  s->cmdTokens[i] = strtok(s->cmd, " ");
  while (s->cmdTokens[i++] && i < sizeof(s->cmdTokens)) {
    s->cmdTokens[i] = strtok(NULL, " ");
  }
}

/*
 * Read a single character.
 */
char readChar() {
  char c = getchar();
  while (getchar() != '\n')
    ;
  return c;
}

#define SZ 16

/*
 * Execute a child process sequentially with the parent.
 */
static int seq(CloshState const *const s) {
  int pid;
  int status;
  int ok = 1;

  for (int i = 0; i < s->count; i++) {
    pid = fork();
    if (pid == 0) {
      printf("pid: %d \n", getpid());
      if (execvp(*s->cmdTokens, s->cmdTokens) == -1) {
        ok = 0;
      }
    } else {
      if (s->timeout == 0) {      // no timeout
        waitpid(pid, &status, 0); // wait one by one
      } else {
        start_timer(s->timeout);
        waitpid_or_EINTR(&status);
      }
    }
  }
  return ok;
}

/*
 * Execute a child process in parallel
 */
static int par(CloshState const *const s) {
  int pid;
  int ok = 1;

  for (int i = 0; i < s->count; ++i) { // fork children
    pid = fork();
    if (pid == 0) {
      printf("pid: %d \n", getpid());
      if (execvp(*s->cmdTokens, s->cmdTokens) == -1) {
        ok = 0;
      }
    }
  }

  // waitpid(-1, NULL, 0) only wait for any one.
  if (pid != 0) { // this branch is parent only.
    pid_t wpid;
    int status = 0;
    if (s->timeout == 0) {               // no timeout
      while ((wpid = wait(&status)) > 0) // this ensure wait for all.
        ;
    } else {
      start_timer(s->timeout);
      waitpid_or_EINTR(&status);
    }
  }

  return ok;
}
#undef SZ

/*
 * Run the command, using either parallel or sequential execution.
 */
int run(CloshState const *const s) {
  if (s->parallel)
    return par(s);
  else
    return seq(s);
}

/*
 * Handles one interaction between the shell and the user.
 */
void rep(CloshState *s) {
  printf("closh> ");
  fgets(s->cmd, sizeof(s->cmd), stdin);

  // nothing was read
  if (s->cmd[0] == '\n')
    return;

  // split cmd into cmd + arguments
  readTokens(s);

  do {
    printf("  count> ");
    s->count = readChar() - '0';
  } while (s->count <= 0 || s->count > 9);

  printf("  [p]arallel or [s]equential");
  s->parallel = (unsigned int)(readChar() == 'p');

  do {
    printf("  timeout> ");
    s->timeout = readChar() - '0';
  } while (s->timeout < 0 || s->timeout > 9);

  if (!run(s)) {
    printf("Can't execute %s\n", s->cmdTokens[0]);
  }
  exit(1);
}

int main() {
  CloshState s;
  while (1)
    rep(&s);
  return 0;
}
