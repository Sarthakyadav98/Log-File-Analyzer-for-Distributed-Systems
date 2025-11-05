#include "include/log_parser.h"
#include "include/analyzer_utils.h"
#include <iostream>
using namespace std;

int main() {
    // Test with a small log file
    vector<string> files = {"data/clean/log_small.txt"};
    vector<string> logs = readMultipleLogs(files);
    
    cout << "Testing new features with " << logs.size() << " log entries...\n";
    
    // Test serial analysis with new features
    auto result = analyzeLogsSerial(logs);
    
    cout << "\n=== TESTING NEW FEATURES ===\n";
    displayResults(result, logs);
    
    return 0;
}