#!/usr/bin/env python3
"""
Generate log files of different sizes for benchmarking
"""

import random
import datetime

# IP address pool
ips = [f"192.168.0.{i}" for i in range(1, 51)]

# Log messages pool
info_messages = [
    "User login successful",
    "Service started",
    "Request completed",
    "Data synchronized",
    "Cache updated",
    "Connection established"
]

error_messages = [
    "Database connection failed",
    "Timeout occurred",
    "File not found",
    "Permission denied",
    "Memory allocation failed",
    "Service unavailable"
]

warning_messages = [
    "Disk usage high",
    "Memory usage high",
    "Slow response time",
    "Cache miss rate increasing",
    "Network latency detected"
]

debug_messages = [
    "Debugging mode enabled",
    "Debugging trace point",
    "Variable dump requested",
    "Performance monitor active"
]

def generate_log_line(log_type, ip, message):
    """Generate a single log line"""
    timestamp = datetime.datetime.now() + datetime.timedelta(
        hours=random.randint(-100, 100),
        minutes=random.randint(0, 59),
        seconds=random.randint(0, 59)
    )
    return f"{timestamp.strftime('%Y-%m-%d %H:%M:%S')} {log_type} {ip} {message}\n"

def generate_log_file(filename, num_lines):
    """Generate a log file with specified number of lines"""
    with open(filename, 'w') as f:
        for _ in range(num_lines):
            log_type = random.choice(['INFO', 'ERROR', 'WARNING', 'DEBUG'])
            ip = random.choice(ips)
            
            if log_type == 'INFO':
                message = random.choice(info_messages)
            elif log_type == 'ERROR':
                message = random.choice(error_messages)
            elif log_type == 'WARNING':
                message = random.choice(warning_messages)
            else:  # DEBUG
                message = random.choice(debug_messages)
            
            f.write(generate_log_line(log_type, ip, message))

def main():
    # Define different sizes for benchmarks
    sizes = {
        'log_small.txt': 100,
        'log_medium.txt': 1000,
        'log_large.txt': 10000,
        'log_xlarge.txt': 50000,
        'log_xxlarge.txt': 100000
    }
    
    print("Generating log files of different sizes...")
    for filename, num_lines in sizes.items():
        print(f"Generating {filename} with {num_lines:,} lines...")
        generate_log_file(f"data/{filename}", num_lines)
    
    print("\nAll log files generated successfully!")

if __name__ == "__main__":
    main()

