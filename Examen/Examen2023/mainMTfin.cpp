#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "utils.hh"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>


// define data structures

// un exemple simple
int data=0;
std::atomic<int> nbwords(0);
std::unordered_map<std::string,int> keyIndex;
std::vector<std::unordered_set<std::string>> files;
std::vector<std::mutex> mutexes;


// example function
static void onWordEncountered(const std::string& word, const std::string& filename) {
    std::cout << "saw " << word << " in " << filename << std::endl;
	keyIndex.insert({word, data});
    data++;
}

static void onWordEncountered2(const std::string& word, const std::string& filename) {
	//std::unique_lock<std::mutex> lk(mt);
	for(auto &paire : keyIndex) {
		if(word == paire.first) {
			std::unique_lock<std::mutex> lk(mutexes[paire.second]);
			auto & ens = files[paire.second];
			auto it = ens.find(filename);
			if(it == ens.end())
				ens.insert(filename);
		}

	}
	nbwords++;
}

int main (int argc, const char **argv) {
	if (argc < 3) {
		std::cerr << "Invoke with keyword file as first argument followed by the files to index." << std::endl;
		std::cerr << "e.g. indexer data/keywords.txt data/*" << std::endl;
	} else {
		std::cout << "Reading keywords from " << argv[1] << std::endl;
		std::cout << "Indexing files " ;
		for (int i=2; i < argc ; i++) {
			std::cout << argv[i];
		}
		std::cout << std::endl;
	}

	int nbThread = argc - 2;
	std::vector<std::thread> threads;
	threads.reserve(nbThread);

	// parse arguments to main
	processFile(argv[1], onWordEncountered);
	std::cout << "Found " << data << " keywords." << std::endl;

	// initialiser le vecteur de mutexs et le vecteur de set de fichiers
	mutexes = std::move(std::vector<std::mutex>(data));
	files = std::move(std::vector<std::unordered_set<std::string>>(data));

	for (int i=2; i < argc ; i++) {
		threads.emplace_back([&, i]() {
			processFile(argv[i], onWordEncountered2);
		});
	}

	for(auto &it : threads) {
		it.join();
	}

	std::cout << "Found " << nbwords << " words." << std::endl;


	// TODO
	
	for (const auto &paire : keyIndex) {
		std::cout << "key:" << paire.first << std::endl;
		std::cout << "Found in : ";
		for (const auto &file : files[paire.second]) {
			std::cout << file << " ";
		}
		std::cout << std::endl;
	}
	

	return 0;
}
