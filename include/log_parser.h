// Handles reading and tokenizing log files
#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

inline std::vector<std::string> readLogFile(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return lines;
    }

    while (std::getline(file, line)) {
        if (!line.empty())
            lines.push_back(line);
    }

    file.close();
    return lines;
}

// Read multiple files and concatenate lines into a single vector (helper used by src)
inline std::vector<std::string> readMultipleLogs(const std::vector<std::string>& filenames) {
    std::vector<std::string> all;
    for (const auto &f : filenames) {
        std::vector<std::string> part = readLogFile(f);
        all.insert(all.end(), part.begin(), part.end());
    }
    return all;
}


#endif
