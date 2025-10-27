# Log Analyzer Algorithm - Pseudocode Documentation

## 1. SERIAL ALGORITHM - Complete Workflow

```
ALGORITHM: Serial Log Analysis
INPUT: List of log file paths
OUTPUT: Statistics (counts, IPs, error messages)

BEGIN Serial_Log_Analysis

    Step 1: File Reading Phase
        FOR each file in log_files:
            OPEN file
            WHILE not EOF:
                READ line
                IF line is not empty:
                    ADD line to logs[]
            CLOSE file
        END FOR
        
    Step 2: Initialization
        INITIALIZE LogStats:
            infoCount = 0
            errorCount = 0
            warningCount = 0
            debugCount = 0
            ipCount = empty HashMap
            errorMessages = empty HashMap
    
    Step 3: Analysis Phase (MAIN LOOP)
        FOR each line in logs:
            
            // Level counting
            IF line contains "INFO":
                infoCount = infoCount + 1
            END IF
            
            IF line contains "ERROR":
                errorCount = errorCount + 1
                
                // Parse error message
                PARSE line: [date, time, level, ip, message]
                IF message is not empty:
                    errorMessages[message] = errorMessages[message] + 1
                END IF
            END IF
            
            IF line contains "WARNING":
                warningCount = warningCount + 1
            END IF
            
            IF line contains "DEBUG":
                debugCount = debugCount + 1
            END IF
            
            // IP address extraction
            PARSE line: [date, time, level, ip]
            IF ip contains '.':
                ipCount[ip] = ipCount[ip] + 1
            END IF
            
        END FOR
        
    Step 4: Post-processing
        // Sort IPs by frequency
        CREATE list of (ip, count) pairs from ipCount
        SORT by count (descending)
        
        // Sort error messages by frequency
        CREATE list of (message, count) pairs from errorMessages
        SORT by count (descending)
    
    Step 5: Display Results
        PRINT "Keyword Frequency"
        PRINT "INFO: ", infoCount
        PRINT "ERROR: ", errorCount
        PRINT "WARNING: ", warningCount
        PRINT "DEBUG: ", debugCount
        
        PRINT "Top IPs"
        FOR i = 0 to min(5, ipCount.size):
            PRINT ip[i], " -> ", count[i], " times"
        END FOR
        
        PRINT "Top Error Messages"
        FOR i = 0 to min(5, errorMessages.size):
            PRINT "\"", message[i], "\" -> ", count[i], " times"
        END FOR

END Serial_Log_Analysis
```

---

## 2. IDENTIFIED PARALLELIZABLE BLOCKS/SEGMENTS

### Analysis of Algorithm Components

**Fully Parallelizable (Independent Operations):**
1. **Keyword Counting** (lines 3 in Step 3)
   - Detection of INFO, ERROR, WARNING, DEBUG keywords
   - Independent operations with accumulation

2. **IP Address Extraction** (lines 4 in Step 3)
   - IP extraction from each line
   - Counting occurrences

3. **Error Message Extraction** (lines 2-6 in Step 3)
   - Parsing error messages
   - Counting occurrences

**Partially Parallelizable (Requires Synchronization):**
4. **HashMap Updates** (within Step 3)
   - Insertions into ipCount and errorMessages
   - Requires critical sections or atomic operations

**Not Parallelizable (Sequential):**
5. **File Reading** (Step 1)
   - I/O bound operation
   - Must complete before analysis

6. **Sorting and Display** (Steps 4-5)
   - Sequential post-processing
   - No benefit from parallelization

---

## 3. PARALLEL PSEUDOCODE

```
ALGORITHM: Parallel Log Analysis (OpenMP-based)
INPUT: List of log file paths, numThreads
OUTPUT: Statistics (counts, IPs, error messages)

BEGIN Parallel_Log_Analysis

    Step 1: File Reading Phase (SEQUENTIAL)
        FOR each file in log_files:
            OPEN file
            WHILE not EOF:
                READ line
                IF line is not empty:
                    ADD line to logs[]
            CLOSE file
        END FOR
        
    Step 2: Initialization
        INITIALIZE LogStats:
            infoCount = 0
            errorCount = 0
            warningCount = 0
            debugCount = 0
            ipCount = empty HashMap (thread-safe access)
            errorMessages = empty HashMap (thread-safe access)
        
        // Thread-local accumulators for reduction
        LOCAL info = 0
        LOCAL error = 0
        LOCAL warning = 0
        LOCAL debug = 0
    
    Step 3: PARALLEL Analysis Phase (MAIN PARALLELIZATION)
        
        // OpenMP Parallel For with Reduction
        #pragma omp parallel for num_threads(numThreads) 
                                reduction(+:info, error, warning, debug)
        FOR i = 0 to logs.size - 1:
            
            LOCAL line = logs[i]
            
            // Level counting (with reduction)
            IF line contains "INFO":
                info = info + 1
            END IF
            
            IF line contains "ERROR":
                error = error + 1
                
                // Parse error message with critical section
                PARSE line: [date, time, level, ip, message]
                IF message is not empty:
                    #pragma omp critical
                    {
                        errorMessages[message] = errorMessages[message] + 1
                    }
                END IF
            END IF
            
            IF line contains "WARNING":
                warning = warning + 1
            END IF
            
            IF line contains "DEBUG":
                debug = debug + 1
            END IF
            
            // IP address extraction with critical section
            PARSE line: [date, time, level, ip]
            IF ip contains '.':
                #pragma omp critical
                {
                    ipCount[ip] = ipCount[ip] + 1
                }
            END IF
            
        END FOR
        
        // Reduction: combine thread-local values
        infoCount = info (automatic reduction)
        errorCount = error (automatic reduction)
        warningCount = warning (automatic reduction)
        debugCount = debug (automatic reduction)
        
    Step 4: Post-processing (SEQUENTIAL)
        // Same as serial version
        CREATE list of (ip, count) pairs from ipCount
        SORT by count (descending)
        
        CREATE list of (message, count) pairs from errorMessages
        SORT by count (descending)
    
    Step 5: Display Results (SEQUENTIAL)
        // Same as serial version
        PRINT "Keyword Frequency"
        PRINT "INFO: ", infoCount
        PRINT "ERROR: ", errorCount
        PRINT "WARNING: ", warningCount
        PRINT "DEBUG: ", debugCount
        
        PRINT "Top IPs"
        FOR i = 0 to min(5, ipCount.size):
            PRINT ip[i], " -> ", count[i], " times"
        END FOR
        
        PRINT "Top Error Messages"
        FOR i = 0 to min(5, errorMessages.size):
            PRINT "\"", message[i], "\" -> ", count[i], " times"
        END FOR

END Parallel_Log_Analysis
```

---

## 4. KEY DIFFERENCES: Serial vs Parallel

| Aspect | Serial Algorithm | Parallel Algorithm |
|--------|-----------------|-------------------|
| **Main Loop** | Sequential FOR each line | Parallel FOR each line (OpenMP) |
| **Keyword Counting** | Direct increment | Reduction operation |
| **IP/Message Counting** | Direct HashMap access | Critical section protection |
| **Thread Safety** | Not required | Required for shared data |
| **Performance** | O(n) sequential | O(n/p) where p = threads |
| **Scalability** | Linear with data size | Better with large datasets |

---

## 5. PARALLELIZATION STRATEGY

### Strategy 1: OpenMP Parallel For
- **Use**: Automatically divides loop iterations among threads
- **Syntax**: `#pragma omp parallel for`
- **Applies to**: Main analysis loop

### Strategy 2: Reduction Operation
- **Use**: Thread-safe accumulation of numeric values
- **Syntax**: `reduction(+:variable)`
- **Applies to**: infoCount, errorCount, warningCount, debugCount
- **Benefit**: Avoids critical sections, faster than atomic operations

### Strategy 3: Critical Sections
- **Use**: Thread-safe access to shared data structures
- **Syntax**: `#pragma omp critical`
- **Applies to**: HashMap insertions (ipCount, errorMessages)
- **Trade-off**: Slight overhead but ensures correctness

---

## 6. PARALLELIZATION OVERHEAD

### Sources of Overhead:
1. **Thread Creation**: Minor overhead at start of parallel region
2. **Load Balancing**: Uneven distribution may cause thread starvation
3. **Critical Sections**: Serialization of HashMap updates
4. **Cache Coherency**: False sharing (minimal in this case)

### Optimizations Applied:
- **Reduction**: Efficient thread-local accumulation
- **Coarse-grained Critical**: Minimal lock contention
- **Static Scheduling**: Predictable work distribution

---

## 7. EXPECTED PERFORMANCE

### Time Complexity:
- **Serial**: O(n) where n = number of log lines
- **Parallel**: O(n/p) where p = number of threads

### Speedup:
- **Ideal**: p-fold speedup (p = numThreads)
- **Actual**: Depends on:
  - Dataset size (larger = better speedup)
  - Critical section contention
  - Overhead vs computation ratio

### Efficiency:
- **Target**: 75-95% (good parallel efficiency)
- **Factors**: Thread synchronization costs, HashMap contention

---

## 8. IMPLEMENTATION DETAILS

### Critical Sections Used:
```cpp
#pragma omp critical
{
    stats.errorMessages[message]++;
    stats.ipCount[ip]++;
}
```
**Why**: HashMap insertions are not thread-safe. Critical sections ensure only one thread modifies the map at a time.

### Reduction Operations:
```cpp
#pragma omp parallel for reduction(+:info, error, warning, debug)
```
**Why**: Automatically handles thread-safe accumulation without locks. Much faster than critical sections for simple increments.

### OpenMP Directives Summary:
1. `#pragma omp parallel for` - Creates threads and divides loop
2. `num_threads(n)` - Controls number of threads
3. `reduction(+:vars)` - Thread-safe accumulation
4. `#pragma omp critical` - Exclusive access to shared resources

---

This pseudocode provides a complete understanding of the serial to parallel transformation in the log analyzer codebase.

