#include "log_preprocessor.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <omp.h>

namespace fs = std::filesystem;

std::vector<LogEntry> preprocessLogFile(const std::string &inputPath, std::size_t &totalLinesOut, std::size_t &validLinesOut) {
    std::ifstream infile(inputPath);
    std::vector<LogEntry> logs;
    totalLinesOut = 0;
    validLinesOut = 0;

    if (!infile.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << inputPath << std::endl;
        return logs;
    }

    std::string line;
    // Format: YYYY-MM-DD HH:MM:SS LEVEL IP MESSAGE
    // Be tolerant to multiple spaces/tabs; IP strictly d{1,3}.d{1,3}.d{1,3}.d{1,3}
    // Also allow DD/MM/YYYY formats.
    std::regex logPattern(
        R"((\d{4}[-/]\d{2}[-/]\d{2}|\d{2}[-/]\d{2}[-/]\d{4})\s+(\d{2}:\d{2}:\d{2})\s+(INFO|ERROR|WARNING|DEBUG)\s+(\d{1,3}(?:\.\d{1,3}){3})\s+(.+))",
        std::regex_constants::icase);
    std::smatch match;

    while (std::getline(infile, line)) {
        ++totalLinesOut;
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
            ++validLinesOut;
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

PreprocessStats preprocessAllLogs(const std::string &inputDir, const std::string &outputDir) {
    if (!fs::exists(outputDir))
        fs::create_directories(outputDir);

    std::vector<fs::path> files;
    for (const auto &file : fs::directory_iterator(inputDir)) {
        if (file.is_regular_file()) {
            files.push_back(file.path());
        }
    }

    PreprocessStats stats;
    stats.totalFiles = files.size();

    double t0 = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic)
    for (std::size_t i = 0; i < files.size(); ++i) {
        const auto &path = files[i];
        std::string inputPath = path.string();
        std::string outputPath = (fs::path(outputDir) / path.filename()).string();

        std::size_t totalLines = 0;
        std::size_t validLines = 0;

        auto logs = preprocessLogFile(inputPath, totalLines, validLines);
        writeProcessedLogs(logs, outputPath);

        #pragma omp critical
        {
            std::cout << "[INFO] Processing: " << inputPath << std::endl;
            std::cout << "[INFO] → Saved cleaned file: " << outputPath
                      << " (" << validLines << " valid entries)" << std::endl;
        }

        #pragma omp atomic update
        stats.totalLines += totalLines;
        #pragma omp atomic update
        stats.validLines += validLines;
    }

    double t1 = omp_get_wtime();
    double elapsed = t1 - t0;

    std::size_t filtered = (stats.totalLines >= stats.validLines) ? (stats.totalLines - stats.validLines) : 0;
    std::cout << "\n[STATS] Preprocessing completed in " << elapsed << " s" << std::endl;
    std::cout << "[STATS] Files: " << stats.totalFiles
              << ", Lines: " << stats.totalLines
              << ", Valid: " << stats.validLines
              << ", Filtered: " << filtered << std::endl;

    return stats;
}
