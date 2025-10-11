#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <ios>
#include "HashMap.h"
#include "HashMapAtomic.h"
#include "FileUtils.h"
#include <thread>
#include <mutex>
#include "HashMapMT.h"
#include "HashMapFine.h"

using namespace std;

int main(int argc, char **argv)
{
        using namespace std::chrono;

        // Allow filename as optional first argument, default to project-root/WarAndPeace.txt
        // Optional second argument is mode (e.g. "freqstd" or "freq").
        // Optional third argument is num_threads (default 4).
        string filename = "../WarAndPeace.txt";
        string mode = "freqstd";
        int num_threads = 4;
        if (argc > 1)
                filename = argv[1];
        if (argc > 2)
                mode = argv[2];
        if (argc > 3)
                num_threads=atoi(argv[3]);
        // ascii to int (atoi)

        // Check if file is readable
        ifstream check(filename, std::ios::binary);
        if (!check.is_open())
        {
                cerr << "Could not open '" << filename << "'. Please provide a readable text file as the first argument." << endl;
                cerr << "Usage: " << (argc > 0 ? argv[0] : "TME3") << " [path/to/textfile] [mode] [num threads]" << endl;
                return 2;
        }
        check.seekg(0, std::ios::end);
        std::streamoff file_size = check.tellg();
        check.close();

        cout << "Preparing to parse " << filename << " (mode=" << mode << " N=" << num_threads << "), containing " << file_size << " bytes" << endl;

        auto start = steady_clock::now();

        std::vector<std::pair<std::string, int>> pairs;

        if (mode == "freqstd") {
                ifstream input(filename, std::ios::binary);
                size_t total_words = 0;
                size_t unique_words = 0;
                std::unordered_map<std::string, int> um;
                std::string word;
                while (input >> word) {
                        word = pr::cleanWord(word);
                        if (!word.empty()) {
                                total_words++;
                                //Avec cette syntaxe y'a la recherche de la clé et l'incrémentation
                                //si elle exite pas on crée une nouvelle et l'entier initialisé par défaut à 0
                                ++um[word];
                        }
                }
                unique_words = um.size();
                pairs.reserve(unique_words);
                for (const auto& p : um) pairs.emplace_back(p);
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

        } else if (mode == "freqstdf") {
                size_t total_words = 0;
                size_t unique_words = 0;
                std::unordered_map<std::string, int> um;
                pr::processRange(filename, 0, file_size, [&](const std::string& word) {
                        total_words++;
                        um[word]++;
                });
                unique_words = um.size();
                pairs.reserve(unique_words);
                for (const auto& p : um) pairs.emplace_back(p);
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

        } else if (mode == "freq") {
                size_t total_words = 0;
                size_t unique_words = 0;
                HashMap<std::string, int> hm;
                pr::processRange(filename, 0, file_size, [&](const std::string& word) {
                        total_words++;
                        hm.incrementFrequency(word);
                });
                pairs = hm.toKeyValuePairs();
                unique_words = pairs.size();
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        }else if (mode == "partition") {
                size_t total_words = 0;
                size_t unique_words = 0;
                std::unordered_map<std::string, int> um;
                auto v = pr::partition(filename, file_size, num_threads);
                for(int i = 0; i < (int) (v.size() - 1); i++) {
                        pr::processRange(filename, v[i], v[i+1], [&](const std::string& word) {
                                total_words++;
                                um[word]++;
                        });
                }
                unique_words = um.size();
                pairs.reserve(unique_words);
                for (const auto& p : um) pairs.emplace_back(p);
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        } else if (mode == "mt_naive") {
                size_t total_words = 0;
                size_t unique_words = 0;
                std::unordered_map<std::string, int> um;
                auto v = pr::partition(filename, file_size, num_threads);
                std::vector<std::thread> th;
                th.reserve(num_threads);

                /* ou plus flexible 
                for(int i = 0; i < v.size() - 1; i++) {
                     th.emplace_back([&, i]() {
                                pr::processRange(filename, v[i], v[i+1], [&](const std::string& word) {
                                total_words++;
                                um[word]++;
                        }
                                });   
                }

                */
                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back(pr::processRange, std::cref(filename), v[i], v[i+1],[&](const std::string& word) {
                                total_words++;
                                um[word]++;
                        });   
                }

                for(auto & t: th) {
                    t.join();
                }

                unique_words = um.size();
                pairs.reserve(unique_words);
                for (const auto& p : um) pairs.emplace_back(p);
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        } else if (mode == "mt_hnaive") {
                size_t total_words = 0;
                size_t unique_words = 0;
                HashMap<std::string, int> hm;
                auto v = pr::partition(filename, file_size, num_threads);
                std::vector<std::thread> th;
                th.reserve(num_threads);
                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back([&, i]() {
                                pr::processRange(filename, v[i], v[i+1], [&](const std::string& word) {
                                total_words++;
                                hm.incrementFrequency(word, 1);
                                });
                        });   
                }

                for(auto & t: th) {
                    t.join();
                }

                pairs = hm.toKeyValuePairs();
                unique_words = pairs.size();
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        } else if (mode == "mt_atomic") {
                std::atomic<int> total_words = 0;
                size_t unique_words = 0;
                std::unordered_map<std::string, int> um;
                auto v = pr::partition(filename, file_size, num_threads);
                std::vector<std::thread> th;
                th.reserve(num_threads);

                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back(pr::processRange, std::cref(filename), v[i], v[i+1],[&](const std::string& word) {
                                total_words++;
                                um[word]++;
                        });   
                }

                for(auto & t: th) {
                    t.join();
                }

                unique_words = um.size();
                pairs.reserve(unique_words);
                for (const auto& p : um) pairs.emplace_back(p);
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

                std::cout << "Nb des mots total " << total_words << std::endl; 
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        }  else if (mode == "mt_atomic_max") {
                std::atomic<int> total_words = 0;
                size_t unique_words = 0;
                HashMapAtomic<std::string,int> hm;
                auto v = pr::partition(filename, file_size, num_threads);
                std::vector<std::thread> th;
                th.reserve(num_threads);
                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back([&, i]() {
                                pr::processRange(
                                                filename,
                                                v[i], v[i+1], 
                                                [&](const std::string& word) {
                                                        total_words++;
                                                        hm.incrementFrequency(word, 1);
                                                } 
                                               );
                        });   
                }

                for(auto & t: th) {
                    t.join();
                }

                pairs = hm.toKeyValuePairs();
                unique_words = pairs.size();
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

                std::cout << "Nb des mots uniques " << unique_words << std::endl; 
        }  else if (mode == "mt_mutex") {
                int total_words = 0;
                size_t unique_words = 0;
                std::unordered_map<std::string, int> um;
                std::mutex mutex;
                auto v = pr::partition(filename, file_size, num_threads);
                std::vector<std::thread> th;
                th.reserve(num_threads);

                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back(pr::processRange, std::cref(filename), v[i], v[i+1],[&](const std::string& word) {
                                // On est pas on std::defer_lock (deuxième arg du constructeur de unique_lock)
                                //d'ou le lock() implicite à la création et le unlock() à la destruction après le scope du emplace_back
                                std::unique_lock<std::mutex> lock(mutex);
                                total_words++;
                                um[word]++;
                        });   
                }

                for(auto & t: th) {
                    t.join();
                }

                unique_words = um.size();
                pairs.reserve(unique_words);
                for (const auto& p : um) pairs.emplace_back(p);
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

                std::cout << "Nb des mots total " << total_words << std::endl; 
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        }  else if (mode == "mt_hmutex") {
                size_t unique_words = 0;
                HashMapMT<std::string,int> hm;
                auto v = pr::partition(filename, file_size, num_threads);
                std::vector<std::thread> th;
                th.reserve(num_threads);
                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back([&, i]() {
                                pr::processRange(
                                                filename,
                                                v[i], v[i+1], 
                                                [&](const std::string& word) {
                                                        hm.incrementFrequency(word, 1);
                                                } 
                                               );
                        });   
                }

                for(auto & t: th) {
                    t.join();
                }

                pairs = hm.toKeyValuePairs();
                unique_words = pairs.size();
                pr::printResults(hm.getTotalWords(), unique_words, pairs, mode + ".freq");
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

                std::cout << "Nb des mots uniques " << unique_words << std::endl; 
        } else if (mode == "mt_hashes") {
                std::atomic<int> total_words = 0;
                size_t unique_words = 0;

                // faut initialiser chaque unordered_map sinon UB
                std::vector<std::unordered_map<std::string, int>> hashes(num_threads);

                auto v = pr::partition(filename, file_size, num_threads);

                std::vector<std::thread> th;
                th.reserve(num_threads);

                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back(
                                [&, i]() {
                                      pr::processRange(
                                                        filename,
                                                        v[i], v[i+1], 
                                                        [&](const std::string& word) {
                                                                hashes[i][word]++;
                                                                total_words++;
                                                        } 
                                               );
                                }
                                   );   
                }

                for(auto & t: th) {
                    t.join();
                }

                // On peut aussi le faire avec notre HashMap maison on invoquant incrementFrequency(word, 1);
                std::unordered_map<std::string, int> fusion;

                for (const auto& local_map : hashes) {
                        for (const auto& pair : local_map) {
                                // ici reagrde si pair.first exite elle rend une ref modifiable
                                //sinon elle le crée avec une valeur 0 comme c'est un int
                                //après elle rend la ref modifiable
                                fusion[pair.first] += pair.second;
                        }
                }

                unique_words = fusion.size();
                pairs.reserve(unique_words);
                for (const auto& p : fusion) pairs.emplace_back(p);
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

                std::cout << "Nb des mots total " << total_words << std::endl; 
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        } else if (mode == "mt_hhashes") {
                std::atomic<int> total_words = 0;
                size_t unique_words = 0;

                // faut initialiser chaque unordered_map sinon UB
                std::vector<HashMap<std::string, int>> hashes(num_threads);

                auto v = pr::partition(filename, file_size, num_threads);

                std::vector<std::thread> th;
                th.reserve(num_threads);

                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back(
                                [&, i]() {
                                      pr::processRange(
                                                        filename,
                                                        v[i], v[i+1], 
                                                        [&](const std::string& word) {
                                                                hashes[i].incrementFrequency(word, 1);
                                                                total_words++;
                                                        } 
                                               );
                                }
                                   );   
                }

                for(auto & t: th) {
                    t.join();
                }

                // On peut aussi le faire avec notre HashMap maison on invoquant incrementFrequency(word, 1);
                HashMap<std::string, int> fusion;

                for (const auto& local_map : hashes) {
                    for (const auto& paire : local_map.toKeyValuePairs()) {       
                        fusion.incrementFrequency(paire.first, paire.second);    
                    }
                }
                // Récupérer les paires fusionnées et puis le size() nous donne le nbr des paires uniques
                auto fused_pairs = fusion.toKeyValuePairs();
                unique_words = fused_pairs.size();

                pairs.reserve(unique_words);
                for (const auto& p : fused_pairs) {
                        pairs.emplace_back(p);
                }
                pr::printResults(total_words, unique_words, pairs, mode + ".freq");

                std::cout << "Nb des mots total " << total_words << std::endl; 
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

        }  else if (mode == "mt_hfine") {
                HashMapFine<std::string,int> hm;
                size_t unique_words = 0;
                auto v = pr::partition(filename, file_size, num_threads);
                std::vector<std::thread> th;
                th.reserve(num_threads);
                for(int i = 0; i < (int) (v.size() - 1); i++) {
                     th.emplace_back([&, i]() {
                                pr::processRange(
                                                filename,
                                                v[i], v[i+1], 
                                                [&](const std::string& word) {
                                                        hm.incrementFrequency(word, 1);
                                                } 
                                               );
                        });   
                }

                for(auto & t: th) {
                    t.join();
                }

                pairs = hm.toKeyValuePairs();
                unique_words = pairs.size();
                pr::printResults(hm.getTotalWords(), unique_words, pairs, mode + ".freq");
                std::cout << "Nb des mots uniques " << unique_words << std::endl; 

                std::cout << "Nb des mots uniques " << unique_words << std::endl; 
        } else {
                cerr << "Unknown mode '" << mode << "'. Supported modes: freqstd, freq, freqstdf" << endl;
                return 1;
        }

        // print a single total runtime for successful runs
        auto end = steady_clock::now();
        cout << "Total runtime (wall clock) : " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

        return 0;
}

