#include "log_preprocessor.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

std::vector<LogEntry> preprocessLogFile(const std::string &inputPath) {
    std::ifstream infile(inputPath);
    std::vector<LogEntry> logs;

    if (!infile.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << inputPath << std::endl;
        return logs;
    }

    std::string line;
    // Handles multiple date-time formats (2025-09-29, 29/09/2025, etc.)
    std::regex logPattern(
        R"((\d{4}[-/]\d{2}[-/]\d{2}|\d{2}[-/]\d{2}[-/]\d{4})\s+(\d{2}:\d{2}:\d{2})\s+(\w+)\s+([\d\.]+)\s+(.*))",
        std::regex_constants::icase
    );
    std::smatch match;

    while (std::getline(infile, line)) {
        // Normalize line endings and trim leading/trailing whitespace
        if (!line.empty() && line.back() == '\r') line.pop_back();
        // trim left
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch){ return !std::isspace(ch); }));
        // trim right
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), line.end());

        if (line.empty()) {
            continue;
        }

        if (std::regex_search(line, match, logPattern)) {
            LogEntry entry;
            // Normalize timestamp (convert DD/MM/YYYY → YYYY-MM-DD)
            std::string date = match[1];
            if (date[2] == '/' && date[5] == '/') {
                entry.timestamp = date.substr(6, 4) + "-" + date.substr(3, 2) + "-" + date.substr(0, 2)
                                  + " " + match[2].str();
            } else {
                entry.timestamp = date + " " + match[2].str();
            }
            entry.level = match[3];
            entry.ip = match[4];
            entry.message = match[5];
            logs.push_back(entry);
        } else {
            std::cerr << "[WARN] Skipped malformed line in " << inputPath << ": " << line << std::endl;
        }
    }

    infile.close();
    return logs;
}

void writeProcessedLogs(const std::vector<LogEntry> &logs, const std::string &outputPath) {
    std::ofstream outfile(outputPath);
    if (!outfile.is_open()) {
        std::cerr << "[ERROR] Could not write to file: " << outputPath << std::endl;
        return;
    }

    for (const auto &log : logs) {
        outfile << log.timestamp << " "
                << log.level << " "
                << log.ip << " "
                << log.message << "\n";
    }
    outfile.close();
}

void preprocessAllLogs(const std::string &inputDir, const std::string &outputDir) {
    if (!fs::exists(outputDir))
        fs::create_directories(outputDir);

    for (const auto &file : fs::directory_iterator(inputDir)) {
        if (file.is_regular_file()) {
            std::string inputPath = file.path().string();
            std::string outputPath = (fs::path(outputDir) / file.path().filename()).string();

            std::cout << "[INFO] Processing: " << inputPath << std::endl;
            auto logs = preprocessLogFile(inputPath);
            writeProcessedLogs(logs, outputPath);
            std::cout << "[INFO] → Saved cleaned file: " << outputPath
                      << " (" << logs.size() << " valid entries)" << std::endl;
        }
    }
}
