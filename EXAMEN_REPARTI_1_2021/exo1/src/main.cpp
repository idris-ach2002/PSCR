#include "LinkedQueue.hh"
/*#include "NaiveBlockingQueue.hh"
#include "LinkedBlockingQueue.hh"*/
#include <iostream>

using namespace pr;


#define N 3

int main () {
	pr::LinkedQueue<std::string> queue (10);

	for (int i=0; i < 12 ; i++) {
		if (queue.put(new std::string("P"))) {
			std::cout << "put" << std::endl;
		} else {
			std::cout << "fail" << std::endl;
		}
	}
	for (int i=0; i < 12 ; i++) {
		std::string * s = queue.take();
		if (s) {
			std::cout << *s << std::endl;
		} else {
			std::cout << "null" << std::endl;
		}
		delete s;
	}
	// NB on doit voir 10 put, 2 fail
	// puis 10 fois "P" et 2 fois "null"
}


