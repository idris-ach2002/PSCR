#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "utils.hh"
#include <vector>
#include <thread>

// define data structures

// un exemple simple
int data=0;
int nbwords=0;
std::unordered_map<std::string,std::unordered_set<std::string>> index;

// example function
static void onWordEncountered(const std::string& word, const std::string& filename) {
    std::cout << "saw " << word << " in " << filename << std::endl;
	std::unordered_set<std::string> ensemble;
	index.insert({word, ensemble});
    data++;
}

static void onWordEncountered2(const std::string& word, const std::string& filename) {
	for(auto &paire : index) {
		if(word == paire.first) {
			auto it = paire.second.find(filename);
			if(it == paire.second.end())
				paire.second.insert(filename);
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

	// parse arguments to main
	processFile(argv[1], onWordEncountered);
	std::cout << "Found " << data << " keywords." << std::endl;

	for (int i=2; i < argc ; i++) {
		processFile(argv[i], onWordEncountered2);
	}
	std::cout << "Found " << nbwords << " words." << std::endl;


	// TODO
	
	for (const auto &paire : index) {
		std::cout << "key:" << paire.first << std::endl;
		std::cout << "Found in : ";
		for (const auto &file : paire.second) {
			std::cout << file << " ";
		}
		std::cout << std::endl;
	}
	

	return 0;
}
