#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>



int main(int argc, char **argv) {

  // Parse command line arguments: find the pipe separator "|"
  // Format: ./pipe cmd1 [args...] | cmd2 [args...]
  int i ;

  for (i = 0; i < argc; i++) {
    if(strcmp("|", argv[i]) == 0) {
      argv[i] = nullptr;
      break;
    }
  }
  // séparation des deux tableau
  std::cout << i << std::endl;
  i++;

  // Create a pipe for inter-process communication
  int fd[2];
  if(pipe(fd) < 0) {
    perror("Création du tube anonyme");
    exit(1);
  }

  // Fork the first child process ; child redirects out to write end of pipe, then exec
  pid_t child1 = fork();
  if(child1 < 0) {
    perror("Fork process 1");
    exit(2);
  }

  if(!child1) {
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    execv(argv[0], argv);
  }

  // Fork the second child process ; child redirects in from read end of pipe, then exec
  pid_t child2 = fork();
  if(child2 < 0) {
    perror("Fork process 2");
    exit(3);
  }

  if(!child2) {
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    execv(argv[i], argv+i);
  }

  // Wait for both children to finish

  waitpid(child1, nullptr, 0);
  waitpid(child2, nullptr, 0);

  return 0;
}
