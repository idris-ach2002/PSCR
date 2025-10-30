#pragma once

#include <iostream>
#include <fstream>
#include <string>

// Define the function pointer type for the onWordEncountered behavior
typedef void (*WordEncounteredFunc)(const std::string& word, const std::string& path);


// Modified processFile function that takes a function pointer
void processFile(const std::string& filepath, WordEncounteredFunc callback) {
    std::ifstream file(filepath);
    std::string word;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return;
    }

    // Read words from the file one by one
    while (file >> word) {
        // Use the function pointer to call the onWordEncountered behavior
        callback(word, filepath);
    }

    file.close();
}

