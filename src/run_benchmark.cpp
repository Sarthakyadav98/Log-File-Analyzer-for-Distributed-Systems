// Purpose: Runs benchmarks on multiple log files and outputs CSV data
// Compile with: g++ -std=c++17 -fopenmp -O2 -I./include run_benchmark.cpp -o run_benchmark

#include "../include/log_parser.h"
#include "../include/analyzer_utils.h"
#include "../include/timer.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>

using namespace std;

struct BenchmarkResult {
    string filename;
    int numLines;
    double serialTime;
    double parallelTime;
    double speedup;
    double efficiency;
};

int main(int argc, char* argv[]) {
    // Define log files to benchmark (in order of increasing size)
    vector<string> logFiles = {
        "data/log1.txt",
        "data/log2.txt",
        "data/log_small.txt",
        "data/log_medium.txt",
        "data/log_large.txt",
        "data/log_xlarge.txt",
        "data/log_xxlarge.txt"
    };
    
    vector<BenchmarkResult> results;
    
    cout << "Log File,NumLines,SerialTime(s),ParallelTime(s),Speedup,Efficiency(%)\n";
    
    for (const auto& filename : logFiles) {
        // Skip files that don't exist
        ifstream testFile(filename);
        if (!testFile.good()) {
            testFile.close();
            continue;
        }
        testFile.close();
        
        vector<string> logs = readMultipleLogs({filename});
        int numLines = logs.size();
        
        if (numLines == 0) continue;
        
        // Warm up
        analyzeLogsSerial(logs);
        analyzeLogsParallel(logs);
        
        // Run serial benchmark
        double serialTime = 0.0;
        int numRuns = 5;
        for (int i = 0; i < numRuns; ++i) {
            Timer t;
            auto result = analyzeLogsSerial(logs);
            serialTime += t.elapsed();
        }
        serialTime /= numRuns;
        
        // Run parallel benchmark
        double parallelTime = 0.0;
        for (int i = 0; i < numRuns; ++i) {
            Timer t;
            auto result = analyzeLogsParallel(logs);
            parallelTime += t.elapsed();
        }
        parallelTime /= numRuns;
        
        BenchmarkResult result;
        result.filename = filename;
        result.numLines = numLines;
        result.serialTime = serialTime;
        result.parallelTime = parallelTime;
        result.speedup = serialTime / parallelTime;
        result.efficiency = (result.speedup / 4.0) * 100.0; // Assuming 4 threads
        
        cout << filename << "," 
             << numLines << ","
             << fixed << setprecision(6) << result.serialTime << ","
             << fixed << setprecision(6) << result.parallelTime << ","
             << fixed << setprecision(2) << result.speedup << ","
             << fixed << setprecision(2) << result.efficiency << "\n";
        
        results.push_back(result);
    }
    
    return 0;
}

