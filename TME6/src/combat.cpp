#include <iostream>     // std::cout, std::endl
#include <signal.h>     // sigaction, sigemptyset, sigaddset, sigprocmask, SIGUSR1, sig_atomic_t
#include <unistd.h>     // fork, getpid, getppid, kill
#include <sys/types.h>  // pid_t (par sécurité)
#include <sys/wait.h>   // waitpid, WIFEXITED, WEXITSTATUS
#include <errno.h>      // errno, ESRCH
#include <stdlib.h>     // exit
#include <util/rsleep.h>


volatile sig_atomic_t PV = 3; //chaque process à son propre PV

void attack_handler(int sig) {
    PV--;
    //std::cout << "je suis " << getpid() << " J'ai reçu un signal [ il me reste " << PV << " ] points de vies" << std::endl;
}

void attaque(pid_t adversaire) {
    struct sigaction sa = {0};
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = attack_handler;
    sigaction(SIGUSR1, &sa, nullptr);

    if(kill(adversaire, SIGUSR1) < 0) { //-1 est renvoyé appel a échoué 
        // ESRCH dans errno le process cible n'existe pas
        if(errno == ESRCH)
            exit(0); // on a gagné
    }

    // s'endormer pendant (0.3 à 1) secondes
    pr::randsleep();
}

void handler_luke(int signal) {
    std::cout << "[Luke] Coup paré [Défendu !]" << std::endl;
}

void defense_luke() {
    // désarmer SIGUSR1  (le signal nous intéresse pas même si on le reçoit )
    //on fait rien on décrémente pas les PV 
    //soit on crée un autre handler pour afficher paie ou default ignoronce
    struct sigaction sa;
    sa.sa_handler = handler_luke; // on ignore les signaux pendant la défense
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, nullptr);

    //masquer les signaux
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, nullptr);

    // s'endormer pendant (0.3 à 1) secondes
    pr::randsleep();
    sigdelset(&set, SIGUSR1);
    // attendre indéfiniment SIGUSR1 (Si on là reçu et il est dans pending on exécute le traitement associé) il sera déliveré
    sigsuspend(&set);

    // restaurer le mask vide
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, nullptr);
}

void defense_vador() {
    // désarmer SIGUSR1  (le signal nous intéresse pas même si on le reçoit )
    //on fait rien on décrémente pas les PV 
    //soit on crée un autre handler pour afficher paie ou default ignoronce
    struct sigaction sa;
    sa.sa_handler = SIG_IGN; // on ignore les signaux pendant la défense
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, nullptr);

    // s'endormer pendant (0.3 à 1) secondes
    pr::randsleep();
}




void combat_luke(pid_t adversaire) {
    while(PV > 0) {
        //défense puis attaque
        //Process courant se défend et attaque l'adversaire
        defense_luke();
        attaque(adversaire);
        std::cout << "je suis " << getpid() << " [ il me reste " << PV << " ] points de vies" << std::endl;
    }

    //défaite
    std::cout << "Défaite pour le process " << getpid() << " durant l'attaque à PROCESS " << adversaire << std::endl;

}

void combat_vador(pid_t adversaire) {
    while(PV > 0) {
        int status;
        //attente non bloquante pour vérifier ou il on est le fils
        // soit 0 ou pid du fils
        int ret = waitpid(adversaire, &status, WNOHANG);
        // si le fils à terminé 
        if (ret == adversaire) {
            if(WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                std::cout << "luke à gagné" << std::endl;
            } else {
                std::cout << "vador à gagné" << std::endl;
            }
            exit(0);
        }
        
        //défense puis attaque
        //Process courant se défend et attaque l'adversaire
        defense_vador();
        attaque(adversaire);
        std::cout << "je suis " << getpid() << " [ il me reste " << PV << " ] points de vies" << std::endl;
    }

    //défaite
    std::cout << "Défaite pour le process " << getpid() << " durant l'attaque à PROCESS " << adversaire << std::endl;

}

int main() {
    std::cout << "Placeholder for combat" << std::endl;

    //Vador (PERE)
    pid_t vador = getpid();
    std::cout << "Père Vador => pid == " << vador << std::endl;
    
    pid_t luke = fork();
    if(luke < 0) perror("fork");


    //luke 
    if(luke == 0) {
        std::cout << "Luke => pid == " << getpid() << std::endl;
       combat_luke(vador);
    } else { // vador
       combat_vador(luke);
    }

    return 0;
}