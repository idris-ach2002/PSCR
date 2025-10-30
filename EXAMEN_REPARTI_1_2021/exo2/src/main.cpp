#include "NaiveBlockingQueue.hh"
//#include "LinkedBlockingQueue.hh"
#include <iostream>
#include <thread>
#include <vector>

using namespace pr;


int main () {
	/*
	Question 5. (2 points) Ecrivez un programme principal qui crée 3 thread producteur et 3 thread
	consommateur, chacun devant respectivement ajouter 2000 ou retirer 2000 éléments de la queue,
	initialisée avec une capacité de 100. Le programme doit ensuite se terminer proprement et sans
	fautes ou fuites mémoire. On utilisera une queue de std :: string; le contenu des string est sans
	importance (on peut simplement mettre des ′′toto′′ dans la queue).
	*/

	pr::NaiveBlockingQueue<std::string> queue (100);
	std::vector<std::thread> prod;
	std::vector<std::thread> cons;
	prod.reserve(3);
	cons.reserve(3);

	for(int i = 0; i < 3; i++) {
		prod.emplace_back([&]() {
			for(int i = 0; i < 2000; i++) {
				queue.put(new std::string("toto"));
			}
		});
	}

	for(int i = 0; i < 3; i++) {
		cons.emplace_back([&]() {
			for(int i = 0; i < 2000; i++) {
				std::string * s = queue.take();
				delete s;
			}
		});
	}


	for(auto & prd : prod) {
		prd.join();
	}

	for(auto & cns : cons) {
		cns.join();
	}

	std::cout << "terminé" << std::endl;
	
	return 0;
}

