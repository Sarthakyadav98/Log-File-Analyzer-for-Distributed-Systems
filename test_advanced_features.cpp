#include "include/log_parser.h"
#include "include/analyzer_utils.h"
#include "include/timer.h"
#include <iostream>
using namespace std;

int main() {
    cout << "=== TESTING ADVANCED FEATURES ===\n";
    
    // Test with multiple log files
    vector<string> files = {"data/clean/log_large.txt", "data/clean/log_xlarge.txt"};
    vector<string> logs = readMultipleLogs(files);
    
    cout << "Loaded " << logs.size() << " log entries for testing...\n";
    
    // Test serial analysis with all new features
    Timer t;
    auto result = analyzeLogsSerial(logs);
    double time = t.elapsed();
    
    cout << "\n=== SERIAL ANALYSIS WITH ADVANCED FEATURES ===\n";
    displayResults(result, logs);
    cout << "\nExecution Time: " << time << " seconds\n";
    
    // Test parallel analysis
    Timer t2;
    auto parallelResult = analyzeLogsParallel(logs);
    double parallelTime = t2.elapsed();
    
    cout << "\n=== PARALLEL ANALYSIS WITH ADVANCED FEATURES ===\n";
    displayResults(parallelResult, logs);
    cout << "\nExecution Time: " << parallelTime << " seconds\n";
    
    cout << "\n=== PERFORMANCE COMPARISON ===\n";
    cout << "Serial Time: " << time << "s\n";
    cout << "Parallel Time: " << parallelTime << "s\n";
    cout << "Speedup: " << (time / parallelTime) << "x\n";
    
    return 0;
}