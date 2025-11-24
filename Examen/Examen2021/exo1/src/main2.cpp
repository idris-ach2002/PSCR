#include "LinkedBlockingQueue.hh"
#include <iostream>
#include <thread>
#include <vector>

using namespace pr;


#define N 3

int main () {
	pr::LinkedBlockingQueue<std::string> queue (100);
    std::vector<std::thread> prod;
    std::vector<std::thread> cons;
    prod.reserve(3);
    cons.reserve(3);

    for(int i = 0; i < 3; i++) {
        prod.emplace_back([&] {
            for(int j = 0; j < 2000; j++) {
                queue.put(new std::string("toto"));
            }
        });
    }

    for(int i = 0; i < 3; i++) {
        cons.emplace_back([&] {
            for(int j = 0; j < 2000; j++) {
                std::string * s = queue.take();
                if (s) {
                    std::cout << *s << std::endl;
                } else {
                    std::cout << "null" << std::endl;
                }
                delete s;
            }
        });
    }

    for(auto &it : prod) {
        it.join();
    }

    std::cout << "Producteurs terminés" << std::endl;

    for(auto &it : cons) {
        it.join();
    }

    std::cout << "Consommateurs terminés" << std::endl;

    return 0;
}


