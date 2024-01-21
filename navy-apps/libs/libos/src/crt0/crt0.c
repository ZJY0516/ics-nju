#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  char *empty[] =  {NULL };
  int argc = *(int *)args;
  char **argv = (char **)(args + 1);
  for (int i = 0; i < argc; i++) {
      printf("libos argv: %s\n", argv[i]);
  }
  environ = empty;
  exit(main(0, empty, empty));
  assert(0);
}
