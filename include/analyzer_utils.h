// Core analysis logic — counting, merging, printing
#ifndef ANALYZER_UTILS_H
#define ANALYZER_UTILS_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <omp.h>

struct LogStats {
    int infoCount = 0;
    int errorCount = 0;
    int warningCount = 0;
    int debugCount = 0;
    std::unordered_map<std::string, int> ipCount;
    std::unordered_map<std::string, int> errorMessages;
};

// --- Serial Analysis ---
inline LogStats analyzeLogsSerial(const std::vector<std::string>& logs) {
    LogStats stats;
    for (const auto& line : logs) {
        if (line.find("INFO") != std::string::npos) stats.infoCount++;
        if (line.find("ERROR") != std::string::npos) {
            stats.errorCount++;
            // Extract error message (text after IP address)
            std::istringstream iss(line);
            std::string date, time, level, ip, message;
            iss >> date >> time >> level >> ip;
            std::getline(iss, message);
            message.erase(0, message.find_first_not_of(" \t")); // trim leading whitespace
            if (!message.empty()) {
                stats.errorMessages[message]++;
            }
        }
        if (line.find("WARNING") != std::string::npos) stats.warningCount++;
        if (line.find("DEBUG") != std::string::npos) stats.debugCount++;
        
        // Extract IP address (assuming format: date time level IP message)
        std::istringstream iss(line);
        std::string date, time, level, ip;
        iss >> date >> time >> level >> ip;
        if (!ip.empty() && ip.find('.') != std::string::npos) {
            stats.ipCount[ip]++;
        }
    }
    return stats;
}

// --- Parallel Analysis ---
inline LogStats analyzeLogsParallel(const std::vector<std::string>& logs, int numThreads = 4) {
    LogStats stats;
    int info = 0, error = 0, warning = 0, debug = 0;
    
    // For thread-safe operations on maps, we'll use critical sections
    std::unordered_map<std::string, int> localIpCount;
    std::unordered_map<std::string, int> localErrorMessages;

    #pragma omp parallel for num_threads(numThreads) reduction(+:info, error, warning, debug)
    for (int i = 0; i < (int)logs.size(); ++i) {
        const auto& line = logs[i];
        if (line.find("INFO") != std::string::npos) info++;
        if (line.find("ERROR") != std::string::npos) {
            error++;
            // Extract error message (text after IP address)
            std::istringstream iss(line);
            std::string date, time, level, ip, message;
            iss >> date >> time >> level >> ip;
            std::getline(iss, message);
            message.erase(0, message.find_first_not_of(" \t")); // trim leading whitespace
            if (!message.empty()) {
                #pragma omp critical
                {
                    stats.errorMessages[message]++;
                }
            }
        }
        if (line.find("WARNING") != std::string::npos) warning++;
        if (line.find("DEBUG") != std::string::npos) debug++;
        
        // Extract IP address (assuming format: date time level IP message)
        std::istringstream iss(line);
        std::string date, time, level, ip;
        iss >> date >> time >> level >> ip;
        if (!ip.empty() && ip.find('.') != std::string::npos) {
            #pragma omp critical
            {
                stats.ipCount[ip]++;
            }
        }
    }

    stats.infoCount = info;
    stats.errorCount = error;
    stats.warningCount = warning;
    stats.debugCount = debug;
    return stats;
}

// --- Display Utility ---
inline void displayResults(const LogStats &stats, int topN = 5) {
    std::cout << "\n=== Keyword Frequency ===\n";
    std::cout << "INFO    : " << stats.infoCount << "\n";
    std::cout << "ERROR   : " << stats.errorCount << "\n";
    std::cout << "WARNING : " << stats.warningCount << "\n";
    std::cout << "DEBUG   : " << stats.debugCount << "\n";

    // Top IPs
    std::vector<std::pair<std::string,int>> ips(stats.ipCount.begin(), stats.ipCount.end());
    std::sort(ips.begin(), ips.end(), [](const auto &a, const auto &b){ return a.second > b.second; });

    std::cout << "\n=== Top IPs ===\n";
    for (int i = 0; i < (int)ips.size() && i < topN; ++i) {
        std::cout << ips[i].first << " -> " << ips[i].second << " times\n";
    }

    // Top Error Messages
    std::vector<std::pair<std::string,int>> em(stats.errorMessages.begin(), stats.errorMessages.end());
    std::sort(em.begin(), em.end(), [](const auto &a, const auto &b){ return a.second > b.second; });

    std::cout << "\n=== Top Error Messages ===\n";
    for (int i = 0; i < (int)em.size() && i < topN; ++i) {
        std::cout << '\"' << em[i].first << "\" -> " << em[i].second << " times\n";
    }
}

#endif
