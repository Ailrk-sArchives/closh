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

void readTokens(CloshState *s) {
  s->cmd[strlen(s->cmd) - 1] = '\0';
  int i = 0;
  s->cmdTokens[i] = strtok(s->cmd, " ");
  while (s->cmdTokens[i++] && i < sizeof(s->cmdTokens)) {
    s->cmdTokens[i] = strtok(NULL, " ");
  }
}

char readChar() {
  char c = getchar();
  while (getchar() != '\n')
    ;
  return c;
}

#define SZ 16
// sequential executioan
static void seq(CloshState const *const s) {
  int pid;
  int status;

  for (int i = 0; i < s->count; i++) {
    pid = fork();
    if (pid == 0) {
      printf("pid: %d \n", getpid());
      execvp(*s->cmdTokens, s->cmdTokens);
    } else {
      if (s->timeout == 0) {      // no timeout
        waitpid(pid, &status, 0); // wait one by one
      } else {
        start_timer(s->timeout);
        waitpid_or_EINTR(&status);
      }
    }
  }
}

// parallel execution.
static void par(CloshState const *const s) {
  int pid;

  for (int i = 0; i < s->count; ++i) { // fork children
    pid = fork();
    if (pid == 0) {
      printf("pid: %d \n", getpid());
      execvp(*s->cmdTokens, s->cmdTokens);
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
}
#undef SZ

// run the command.
void run(CloshState const *const s) {
  if (s->parallel)
    par(s);
  else
    seq(s);
}

// one iteraction between shell and user.
void rep(CloshState *s) {
  printf("closh> ");
  fgets(s->cmd, sizeof(s->cmd), stdin);
  if (s->cmd[0] == '\n')
    return;
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

  run(s);

  printf("Can't execute %s\n", s->cmdTokens[0]);
  exit(1);
}

int main() {
  CloshState s;
  while (1)
    rep(&s);
  return 0;
}
