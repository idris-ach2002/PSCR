#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <algorithm>
#include <vector>
#include "HashMap.h"
#include <unordered_map>

const int NB_BUCKET_INITIAL = 100;

// helper to clean a token (keep original comments near the logic)
static std::string cleanWord(const std::string& raw) {
	// une regex qui reconnait les caractères anormaux (négation des lettres)
	static const std::regex re( R"([^a-zA-Z])");
	// élimine la ponctuation et les caractères spéciaux
	std::string w = std::regex_replace(raw, re, "");
	// passe en lowercase
	std::transform(w.begin(), w.end(), w.begin(), ::tolower);
	return w;
}

static int frequence(const std::vector<std::pair<std::string,int>>& v, const std::string &s) {
	auto it = find_if(v.begin(), v.end(), [&s](const auto& paire_){return paire_.first == s;});
	return it == v.end() ? 0 : it->second;
}

int main(int argc, char** argv) {
	using namespace std;
	using namespace std::chrono;

	// Allow filename as optional first argument, default to project-root/WarAndPeace.txt
	// Optional second argument is mode (e.g. "count" or "unique").
	string filename = "../WarAndPeace.txt";
	string mode = "count";
	if (argc > 1) filename = argv[1];
	if (argc > 2) mode = argv[2];

	ifstream input(filename);
	if (!input.is_open()) {
		cerr << "Could not open '" << filename << "'. Please provide a readable text file as the first argument." << endl;
		cerr << "Usage: " << (argc>0?argv[0]:"TME2") << " [path/to/textfile]" << endl;
		return 2;
	}
	cout << "Parsing " << filename << " (mode=" << mode << ")" << endl;
	
	auto start = steady_clock::now();
	
	// prochain mot lu
	string word;

	if (mode == "count") {
		size_t nombre_lu = 0;
	
		// default counting mode: count total words
		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);

			// word est maintenant "tout propre"
			if (nombre_lu % 100 == 0)
				// on affiche un mot "propre" sur 100
				cout << nombre_lu << ": "<< word << endl;
			nombre_lu++;
		}
	input.close();
	cout << "Finished parsing." << endl;
	cout << "Found a total of " << nombre_lu << " words." << endl;

	} else if (mode == "unique") {
		// skeleton for unique mode
		// before the loop: declare a vector "seen"

		// Optimisation on crée un vector vide et on prévoit 10000 cases 
		// (Allocation en avance) pour éviter des réallocations
		vector<string> seen;
		seen.reserve(10000);

		size_t nombre_lu = 0;

		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);

			// add to seen if it is new
			vector<string>::iterator it = find_if(seen.begin(), seen.end(), [&word](const string& s){return s == word;});
			if(it == seen.end()) {
				seen.emplace_back(word);
			}

			/*if (nombre_lu % 100 == 0)
				// on affiche un mot "propre" sur 100
				cout << nombre_lu << ": "<< word << endl;*/
			nombre_lu++;
		}
		input.close();
		// TODO
		cout << "Finished parsing." << endl;
		cout << "Found a total of " << nombre_lu << " words." << endl;
		cout << "Found " << seen.size() << " unique words." << endl;

	} else if (mode == "freqstd") {
		unordered_map<string, int> seen(NB_BUCKET_INITIAL);

		size_t nombre_lu = 0;

		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);
            auto it = seen.find(word);
            if(it == seen.end())
                seen.emplace(word, 0);
            else
                it->second++;

			nombre_lu++;
		}
		input.close();
		
        vector<pair<string, int>> vect;
        vect.reserve(seen.size());
        for (const pair<string,int> & e : seen) {
            vect.emplace_back(e.first, e.second);
        }


		//Tri par ordre décroissant des fréquences
		std::sort(vect.begin(), vect.end(), [&](const auto &f1, const auto & f2) {return f1.second > f2.second;});

		auto toto = seen.find("toto");
        auto war = seen.find("war");
        auto peace = seen.find("peace");

		cout << "Finished parsing." << endl;
		cout << "Found a total of " << nombre_lu << " words." << endl;
		cout << "Found " << seen.size() << " unique pairs." << endl;
		cout << "Found " << (war != seen.end() ? war->second : 0) << " Occurences de war" << endl;
		cout << "Found " << (peace != seen.end() ? peace->second : 0) << " Occurences de peace" << endl;
		cout << "Found " << (toto != seen.end() ? toto->second : 0) << " Occurences de toto" << endl;
		cout << endl << "Affichage => dix mots les plus fréquents " << endl;
		for(int i = 0; i < 10; i++) {
			const auto & p = vect[i];
			cout << "{Mot : " << p.first << " => Freq : " << p.second << "}" << endl;
		}
	}else {
		// unknown mode: print usage and exit
		cerr << "Unknown mode '" << mode << "'. Supported modes: count, unique" << endl;
		input.close();
		return 1;
	}

	// print a single total runtime for successful runs
	auto end = steady_clock::now();
    cout << "NB_BUCKET_INITIAL Configuration is => " << NB_BUCKET_INITIAL << endl;
	cout << "Total runtime (wall clock) : " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

	return 0;
}


