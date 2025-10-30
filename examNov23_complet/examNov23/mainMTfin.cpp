#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include<mutex>
#include<thread>
#include<vector>
#include<atomic>
#include "utils.hh"

// define data structures
std::unordered_map<std::string, int> indexCle;
std::vector<std::mutex> mutexs;
std::vector<std::unordered_set<std::string>> fileSet;


// un exemple simple
int data=0;
std::atomic<int> nbwords{0};

// example function
static void onWordEncountered(const std::string& word, const std::string& filename) {
    std::cout << "saw " << word << " in " << filename << std::endl;
	indexCle.insert({word, data});
    data++;
}

static void onWordEncountered2(const std::string& word, const std::string& filename) {
	auto it = indexCle.find(word);
	if (it != indexCle.end()) {
		int id = it->second;
		std::lock_guard<std::mutex> lk(mutexs[id]);
		fileSet[id].insert(filename);
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

	// parse arguments to main
	processFile(argv[1], onWordEncountered);
	std::cout << "Found " << data << " keywords." << std::endl;

	// initialise les structures globales maintenant que 'data' est connu
	fileSet = std::vector<std::unordered_set<std::string>>(data);
	mutexs  = std::vector<std::mutex>(data);

	std::vector<std::thread> threads;
	threads.reserve(argc - 1);

	for (int i=2; i < argc ; i++) {
		threads.emplace_back([&,i]() {
			processFile(argv[i], onWordEncountered2);
		});
	}

	for(auto &th : threads) {
		th.join();
	}

	std::cout << "Found " << nbwords << " words." << std::endl;


	for (const auto &paire : indexCle) {
		std::cout << "key:" << paire.first << std::endl;
		std::cout << "Found in : ";
		for (const auto &file : fileSet[paire.second]) {
			std::cout << file << " ";
		}
		std::cout << std::endl;
	}
	

	return 0;
}
