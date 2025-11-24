#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdio>


// Custom mystrdup : allocates with new[], copies string (avoid strdup and free)
char* mystrdup(const char* src);

pid_t fils;

void handler(int signal) {
    //std::cout << "[Process Stopped]" << std::endl;
    const char * msg = "[Process Stopped]\n";
    write(STDOUT_FILENO,msg,strlen(msg));
    kill(SIGINT, fils);
}

void safe_end(int status) {
    std::string msg =
        "[FILS] " + std::to_string(fils) +
        " finished successfully RETURNED " + std::to_string(status) + "\n";

    write(STDOUT_FILENO, msg.c_str(), msg.size());
}

void interrupt_end(int signal) {
    std::string msg =
        "[FILS] " + std::to_string(fils) +
        " interrupted by SIGNAL " + std::to_string(signal) + "\n";

    write(STDOUT_FILENO, msg.c_str(), msg.size());
}


int main() {
    std::string line;
    const char * sortie = "exit";

    while (true) {
        std::cout << "mini-shell> " << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nExiting on EOF (Ctrl-D)." << std::endl;
            break;
        }

        // Break Point [Arrêt propre improvisionnel]
        if(strcmp(sortie, line.c_str()) == 0)  {
            write(STDOUT_FILENO,"HOME EXIT\n",strlen("HOME EXIT\n"));
            break;
        }

        if (line.empty()) continue;

        // Simple parsing: split by spaces using istringstream
        std::istringstream iss(line);
        std::vector<std::string> args;
        std::string token;
        while (iss >> token) {
            args.push_back(token);
        }
        if (args.empty()) continue;

        // Prepare C-style argv: allocate and mystrdup
        char** argv = new char*[args.size()];
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = mystrdup(args[i].c_str());
        }

        // On ajoute une gestion d'arrêt pour le process fils 
        // toute on gardant notre shell   ====>  (Or gros une rediriction du signal)
        struct sigaction sa;
        sa.sa_flags = 0;
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGINT, &sa, nullptr);

        // Your code for fork/exec/wait/signals goes here...
        fils = fork();

        if(fils < 0) {
            perror("Error fork");
            exit(EXIT_FAILURE);
        }

        if(fils == 0) { // fils
            std::cout << "FILS " << getpid() << std::endl;
            execvp(argv[0], argv);
            // Unreachable si execvp() ne retourne pas une erreur
            perror("execvp");
            exit(EXIT_FAILURE);
        }else { // père
            // attente indéfinie pour éviter l'état Zombie si on interrompt le process FILS
            while(true) {
                int status;
                pid_t pid = waitpid(fils, &status, WNOHANG);
                if(pid < 0) {
                    perror("ERREUR ATTENTE DU FILS");
                }

                // Fils à terminé Soit proprement ou forcé
                if(pid == fils) {
                    if(WIFEXITED(status)) {
                        safe_end(WEXITSTATUS(status));
                    }else if(WIFSIGNALED(status)) {
                        interrupt_end(WTERMSIG(status));
                    }
                    break; // mais pas exit sinon on quitte même notre shell 
                }

                // Le cas du pid == 0 (Fils en cours d'exécution nous intééresse pas ici)
            }
        }

        // cleanup argv allocations
        for (size_t i = 0; i < args.size(); ++i) {
            delete[] argv[i];
        }
        delete[] argv;
    }
    return 0;
}



char* mystrdup(const char* src) {
    if (src == nullptr) return nullptr;
    size_t len = strlen(src) + 1;  // +1 for null terminator
    char* dest = new char[len];
    memcpy(dest, src, len);  // Or strcpy if preferred
    return dest;
}
