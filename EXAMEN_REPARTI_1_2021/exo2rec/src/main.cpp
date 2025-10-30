#include "LinkedBlockingQueue.hh"
#include <iostream>
#include <thread>
#include <vector>

using namespace pr;


int main () {
	pr::LinkedBlockingQueue<std::string> queue (100);
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

	std::cout << "terminé taille File " << queue.size() << std::endl;
	
	return 0;
}

