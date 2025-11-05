// Core analysis logic — counting, merging, printing
#ifndef ANALYZER_UTILS_H
#define ANALYZER_UTILS_H

#include <string>
#include <unordered_map>
#include <map>
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
    std::unordered_map<std::string, int> warningMessages;
    std::map<std::string, int> timeSlotCount;  // "YYYY-MM-DD HH" -> count
    std::string peakTimeSlot;
    int peakTimeSlotCount = 0;
};

// Helper function to extract time slot from log line
inline std::string extractTimeSlot(const std::string& line) {
    // Extract timestamp from log line format: "YYYY-MM-DD HH:MM:SS LEVEL IP message"
    if (line.length() >= 13) {
        return line.substr(0, 13);  // "YYYY-MM-DD HH"
    }
    return "";
}

// --- Serial Analysis ---
inline LogStats analyzeLogsSerial(const std::vector<std::string>& logs) {
    LogStats stats;
    for (const auto& line : logs) {
        // Extract time slot for load analysis
        std::string timeSlot = extractTimeSlot(line);
        if (!timeSlot.empty()) {
            stats.timeSlotCount[timeSlot]++;
        }
        
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
        if (line.find("WARNING") != std::string::npos) {
            stats.warningCount++;
            // Extract warning message (text after IP address)
            std::istringstream iss(line);
            std::string date, time, level, ip, message;
            iss >> date >> time >> level >> ip;
            std::getline(iss, message);
            message.erase(0, message.find_first_not_of(" \t")); // trim leading whitespace
            if (!message.empty()) {
                stats.warningMessages[message]++;
            }
        }
        if (line.find("DEBUG") != std::string::npos) stats.debugCount++;
        
        // Extract IP address (assuming format: date time level IP message)
        std::istringstream iss(line);
        std::string date, time, level, ip;
        iss >> date >> time >> level >> ip;
        if (!ip.empty() && ip.find('.') != std::string::npos) {
            stats.ipCount[ip]++;
        }
    }
    
    // Find peak time slot
    if (!stats.timeSlotCount.empty()) {
        auto maxSlot = std::max_element(stats.timeSlotCount.begin(), stats.timeSlotCount.end(),
                                       [](const auto &a, const auto &b) { return a.second < b.second; });
        stats.peakTimeSlot = maxSlot->first;
        stats.peakTimeSlotCount = maxSlot->second;
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
        
        // Extract time slot for load analysis
        std::string timeSlot = extractTimeSlot(line);
        if (!timeSlot.empty()) {
            #pragma omp critical
            {
                stats.timeSlotCount[timeSlot]++;
            }
        }
        
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
        if (line.find("WARNING") != std::string::npos) {
            warning++;
            // Extract warning message (text after IP address)
            std::istringstream iss(line);   
            std::string date, time, level, ip, message;
            iss >> date >> time >> level >> ip;
            std::getline(iss, message);
            message.erase(0, message.find_first_not_of(" \t")); // trim leading whitespace
            if (!message.empty()) {
                #pragma omp critical
                {
                    stats.warningMessages[message]++;
                }
            }
        }
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
    
    // Find peak time slot
    if (!stats.timeSlotCount.empty()) {
        auto maxSlot = std::max_element(stats.timeSlotCount.begin(), stats.timeSlotCount.end(),
                                       [](const auto &a, const auto &b) { return a.second < b.second; });
        stats.peakTimeSlot = maxSlot->first;
        stats.peakTimeSlotCount = maxSlot->second;
    }
    
    return stats;
}

// Function to analyze peak hour in detail
inline LogStats analyzePeakHour(const std::vector<std::string>& logs, const std::string& peakTimeSlot) {
    LogStats peakStats;
    
    for (const auto& line : logs) {
        // Check if this log entry is from the peak time slot
        if (line.length() >= 13 && line.substr(0, 13) == peakTimeSlot) {
            if (line.find("INFO") != std::string::npos) peakStats.infoCount++;
            if (line.find("ERROR") != std::string::npos) {
                peakStats.errorCount++;
                // Extract error message
                std::istringstream iss(line);
                std::string date, time, level, ip, message;
                iss >> date >> time >> level >> ip;
                std::getline(iss, message);
                message.erase(0, message.find_first_not_of(" \t"));
                if (!message.empty()) {
                    peakStats.errorMessages[message]++;
                }
            }
            if (line.find("WARNING") != std::string::npos) {
                peakStats.warningCount++;
                // Extract warning message
                std::istringstream iss(line);
                std::string date, time, level, ip, message;
                iss >> date >> time >> level >> ip;
                std::getline(iss, message);
                message.erase(0, message.find_first_not_of(" \t"));
                if (!message.empty()) {
                    peakStats.warningMessages[message]++;
                }
            }
            if (line.find("DEBUG") != std::string::npos) peakStats.debugCount++;
            
            // Extract IP address
            std::istringstream iss(line);
            std::string date, time, level, ip;
            iss >> date >> time >> level >> ip;
            if (!ip.empty() && ip.find('.') != std::string::npos) {
                peakStats.ipCount[ip]++;
            }
        }
    }
    
    return peakStats;
}

// --- Display Utility ---
inline void displayResults(const LogStats &stats, const std::vector<std::string>& logs = {}, int topN = 5) {
    // Calculate total logs processed
    int totalLogs = stats.infoCount + stats.errorCount + stats.warningCount + stats.debugCount;
    std::cout << "Total logs processed: " << totalLogs << "\n";
    
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

    // Top Warning Messages
    std::vector<std::pair<std::string,int>> wm(stats.warningMessages.begin(), stats.warningMessages.end());
    std::sort(wm.begin(), wm.end(), [](const auto &a, const auto &b){ return a.second > b.second; });

    std::cout << "\n=== Top Warning Messages ===\n";
    for (int i = 0; i < (int)wm.size() && i < topN; ++i) {
        std::cout << '\"' << wm[i].first << "\" -> " << wm[i].second << " times\n";
    }

    // Peak Load Time Analysis
    if (!stats.peakTimeSlot.empty()) {
        std::cout << "\n=== Peak Load Time ===\n";
        std::cout << stats.peakTimeSlot << ":00 → " << stats.peakTimeSlotCount << " log entries\n";
        
        // If logs are provided, do detailed peak hour analysis
        if (!logs.empty()) {
            LogStats peakStats = analyzePeakHour(logs, stats.peakTimeSlot);
            
            std::cout << "\n=== Peak Hour Analysis (" << stats.peakTimeSlot << ":00 - " 
                      << stats.peakTimeSlot.substr(11, 2) << ":59) ===\n";
            
            // Top IPs in peak hour
            std::vector<std::pair<std::string,int>> peakIps(peakStats.ipCount.begin(), peakStats.ipCount.end());
            std::sort(peakIps.begin(), peakIps.end(), [](const auto &a, const auto &b){ return a.second > b.second; });
            
            std::cout << "Top IPs: ";
            for (int i = 0; i < (int)peakIps.size() && i < 3; ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << peakIps[i].first << " (" << peakIps[i].second << ")";
            }
            std::cout << "\n";
            
            // Top Error Messages in peak hour
            std::vector<std::pair<std::string,int>> peakErrors(peakStats.errorMessages.begin(), peakStats.errorMessages.end());
            std::sort(peakErrors.begin(), peakErrors.end(), [](const auto &a, const auto &b){ return a.second > b.second; });
            
            if (!peakErrors.empty()) {
                std::cout << "Top Error Messages: ";
                for (int i = 0; i < (int)peakErrors.size() && i < 2; ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << "\"" << peakErrors[i].first << "\" (" << peakErrors[i].second << ")";
                }
                std::cout << "\n";
            }
            
            // Top Warning Messages in peak hour
            std::vector<std::pair<std::string,int>> peakWarnings(peakStats.warningMessages.begin(), peakStats.warningMessages.end());
            std::sort(peakWarnings.begin(), peakWarnings.end(), [](const auto &a, const auto &b){ return a.second > b.second; });
            
            if (!peakWarnings.empty()) {
                std::cout << "Top Warning Messages: ";
                for (int i = 0; i < (int)peakWarnings.size() && i < 2; ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << "\"" << peakWarnings[i].first << "\" (" << peakWarnings[i].second << ")";
                }
                std::cout << "\n";
            }
        }
    }
}

#endif
