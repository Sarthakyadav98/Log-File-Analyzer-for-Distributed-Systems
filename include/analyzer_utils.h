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
#include <fstream>
#include <cmath>
#include <numeric>
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
    std::map<std::string, int> stressScorePerHour;  // "YYYY-MM-DD HH" -> stress score
    std::string peakTimeSlot;
    int peakTimeSlotCount = 0;
    double avgStressScore = 0.0;
    int peakStressScore = 0;
    std::string peakStressTimeSlot;
    std::vector<std::string> anomalousTimeSlots;
};

// Helper function to extract time slot from log line
inline std::string extractTimeSlot(const std::string& line) {
    // Extract timestamp from log line format: "YYYY-MM-DD HH:MM:SS LEVEL IP message"
    if (line.length() >= 13) {
        return line.substr(0, 13);  // "YYYY-MM-DD HH"
    }
    return "";
}

// Helper function to get stress score for log level
inline int getStressScore(const std::string& level) {
    if (level == "ERROR") return 3;
    if (level == "WARNING") return 2;
    if (level == "INFO") return 1;
    if (level == "DEBUG") return 1;
    return 0;
}

// Helper function to extract log level from line
inline std::string extractLogLevel(const std::string& line) {
    std::istringstream iss(line);
    std::string date, time, level;
    iss >> date >> time >> level;
    return level;
}

// Function to calculate stress scores and detect anomalies
inline void calculateStressAndAnomalies(LogStats& stats, const std::vector<std::string>& logs) {
    // Calculate stress scores per hour
    for (const auto& line : logs) {
        std::string timeSlot = extractTimeSlot(line);
        std::string level = extractLogLevel(line);
        if (!timeSlot.empty()) {
            stats.stressScorePerHour[timeSlot] += getStressScore(level);
        }
    }
    
    // Find peak stress score
    if (!stats.stressScorePerHour.empty()) {
        auto maxStress = std::max_element(stats.stressScorePerHour.begin(), stats.stressScorePerHour.end(),
                                         [](const auto &a, const auto &b) { return a.second < b.second; });
        stats.peakStressScore = maxStress->second;
        stats.peakStressTimeSlot = maxStress->first;
    }
    
    // Calculate average stress score
    if (!stats.stressScorePerHour.empty()) {
        int totalStress = 0;
        for (const auto& pair : stats.stressScorePerHour) {
            totalStress += pair.second;
        }
        stats.avgStressScore = static_cast<double>(totalStress) / stats.stressScorePerHour.size();
    }
    
    // Detect anomalies (simple heuristic: count > avg + 2 * std_dev)
    if (stats.timeSlotCount.size() > 1) {
        std::vector<int> counts;
        for (const auto& pair : stats.timeSlotCount) {
            counts.push_back(pair.second);
        }
        
        double mean = std::accumulate(counts.begin(), counts.end(), 0.0) / counts.size();
        double variance = 0.0;
        for (int count : counts) {
            variance += (count - mean) * (count - mean);
        }
        variance /= counts.size();
        double stdDev = std::sqrt(variance);
        
        double threshold = mean + 2 * stdDev;
        
        for (const auto& pair : stats.timeSlotCount) {
            if (pair.second > threshold) {
                stats.anomalousTimeSlots.push_back(pair.first);
            }
        }
    }
}

// Function to save results to CSV
inline void saveResultsToCSV(const LogStats& stats, const std::string& filename = "results/output/performance_report.csv") {
    // Create directory if it doesn't exist
    std::string dir = filename.substr(0, filename.find_last_of('/'));
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create CSV file: " << filename << std::endl;
        return;
    }
    
    // Write header
    file << "TimeSlot,LogCount,StressScore,IsAnomaly\n";
    
    // Write data
    for (const auto& pair : stats.timeSlotCount) {
        const std::string& timeSlot = pair.first;
        int logCount = pair.second;
        int stressScore = 0;
        
        auto stressIt = stats.stressScorePerHour.find(timeSlot);
        if (stressIt != stats.stressScorePerHour.end()) {
            stressScore = stressIt->second;
        }
        
        bool isAnomaly = std::find(stats.anomalousTimeSlots.begin(), 
                                  stats.anomalousTimeSlots.end(), 
                                  timeSlot) != stats.anomalousTimeSlots.end();
        
        file << timeSlot << "," << logCount << "," << stressScore << "," << (isAnomaly ? "1" : "0") << "\n";
    }
    
    file.close();
    std::cout << "\nResults saved to: " << filename << std::endl;
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
    
    // Calculate stress scores and detect anomalies
    calculateStressAndAnomalies(stats, logs);
    
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
    
    // Calculate stress scores and detect anomalies
    calculateStressAndAnomalies(stats, logs);
    
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
    
    // System Stress Score Analysis
    if (!stats.stressScorePerHour.empty()) {
        std::cout << "\n=== System Stress Score ===\n";
        std::cout << "Avg: " << static_cast<int>(stats.avgStressScore) 
                  << " | Peak: " << stats.peakStressScore;
        
        if (stats.avgStressScore > 0) {
            double spikePercent = ((stats.peakStressScore - stats.avgStressScore) / stats.avgStressScore) * 100;
            std::cout << " (↑" << static_cast<int>(spikePercent) << "% spike)";
        }
        std::cout << "\n";
        
        if (!stats.peakStressTimeSlot.empty()) {
            std::cout << "Peak Stress Time: " << stats.peakStressTimeSlot << ":00\n";
        }
    }
    
    // Anomaly Detection Results
    if (!stats.anomalousTimeSlots.empty()) {
        std::cout << "\n=== Anomalous Time Periods ===\n";
        std::cout << "Detected " << stats.anomalousTimeSlots.size() << " anomalous time slots:\n";
        for (const auto& timeSlot : stats.anomalousTimeSlots) {
            auto countIt = stats.timeSlotCount.find(timeSlot);
            int count = (countIt != stats.timeSlotCount.end()) ? countIt->second : 0;
            std::cout << "  " << timeSlot << ":00 → " << count << " log entries (anomaly)\n";
        }
    }
    
    // Save results to CSV
    saveResultsToCSV(stats);
}

#endif
