#include <sys/wait.h>
#include <iostream>
#include <unistd.h>

int main() {
pid_t pid = fork();
if (pid < 0) { perror("fork"); return 1; }
if (pid == 0) { // fils
std::cout << "FILS: pid=" << getpid() << '\n';
sleep(5); return 2;
} else { // pere
int status;
while (true) {
pid_t res = waitpid(pid, &status, WNOHANG);
if (res == 0) { std::cout << "PERE: fils pas termine\n"; sleep(1);}
else if (res == pid && WIFEXITED(status)) {
std::cout << "PERE: fils " << pid << " termine, status=" <<
WEXITSTATUS(status) << '\n';
return 0;
} else { perror("waitpid"); return 1; }
}
}
}