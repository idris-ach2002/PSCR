#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>   // pour errno

constexpr size_t BUFFER_SIZE = 4096;

void cat_fd(int fd) {
    char buffer[BUFFER_SIZE];

    while (true) {
        ssize_t n = read(fd, buffer, BUFFER_SIZE);
        if (n == 0) {
            // EOF
            break;
        }
        if (n < 0) {
            perror("read");
            break;
        }

        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(STDOUT_FILENO, buffer + written, n - written);
            if (w < 0) {
                perror("write");
                exit(1);
            }
            written += w;
        }
    }
}
/*
Par exemple :

        ls | ./mon_cat

Ici, l’entrée standard de mon_cat n’est plus un fichier, mais un tube anonyme (pipe).

Dans notre implémentation, quand il n’y a pas d’arguments, on fait simplement :

        cat_fd(STDIN_FILENO);


Enfin toute à fait oui, cat fonctionne correctement avec un tube en entrée standard, car il ne dépend que
 de read/write qui marchent aussi bien sur des fichiers que sur des pipes.
  La redirection par le shell (ou dup2 dans l’exemple avec pipe)
 se contente de changer le descripteur de fichier derrière STDIN_FILENO,
  sans que cat ait besoin de le savoir
*/
int main(int argc, char *argv[]) {
    // Cas 1 : aucun argument → lire depuis stdin
    if (argc == 1) {
        cat_fd(STDIN_FILENO);
        return 0;
    }

    // Cas 2 : arguments → chaque fichier dans l'ordre
    for (int i = 1; i < argc; ++i) {
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            // message d'erreur sur stderr, puis on continue
            std::cerr << "cat: " << argv[i] << ": " << std::strerror(errno) << "\n";
            continue;
        }

        cat_fd(fd);

        if (close(fd) == -1) {
            perror("close");
        }
    }

    return 0;
}
