#include "../include/log_parser.h"
#include "../include/analyzer_utils.h"
#include "../include/timer.h"
#include <iostream>
using namespace std;

int main() {
    int choice;
    cout << "Choose mode:\n1. Serial\n2. Parallel\n> ";
    cin >> choice;

    vector<string> files = {"../data/log1.txt", "../data/log2.txt"};
    vector<string> logs = readMultipleLogs(files);

    Timer t;
    LogStats result;

    if (choice == 1) result = analyzeLogsSerial(logs);
    else result = analyzeLogsParallel(logs);

    double time = t.elapsed();
    displayResults(result);
    cout << "\nExecution Time: " << time << " seconds\n";
    return 0;
}
