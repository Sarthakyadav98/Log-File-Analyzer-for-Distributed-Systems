#!/usr/bin/env python3
"""
Plot performance comparison between Serial and Parallel implementations
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import subprocess
import os

def run_benchmark():
    """Run the benchmark executable and collect data"""
    print("Running benchmark...")
    
    # Change to project directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.join(script_dir, '..')
    os.chdir(project_dir)
    
    # Run the benchmark
    try:
        # Try Windows executable first, then Unix
        exe_path = 'bin/run_benchmark.exe' if os.path.exists('bin/run_benchmark.exe') else 'bin/run_benchmark'
        result = subprocess.run([exe_path], capture_output=True, text=True, check=True)
        output = result.stdout
        
        # Save to CSV
        with open('results/benchmark_data.csv', 'w') as f:
            f.write(output)
        
        print("Benchmark data saved to results/benchmark_data.csv")
        return output
    except subprocess.CalledProcessError as e:
        print(f"Error running benchmark: {e}")
        print(f"STDERR: {e.stderr}")
        return None
    except FileNotFoundError:
        print("Warning: run_benchmark executable not found. Using existing benchmark data.")
        return None

def load_data():
    """Load benchmark data from CSV"""
    try:
        df = pd.read_csv('results/benchmark_data.csv')
        return df
    except FileNotFoundError:
        print("Error: benchmark_data.csv not found. Please run the benchmark first.")
        return None

def plot_performance_comparison(df):
    """Plot serial vs parallel performance"""
    plt.figure(figsize=(12, 6))
    
    plt.plot(df['NumLines'], df['SerialTime(s)'], 'b-o', label='Serial', linewidth=2, markersize=8)
    plt.plot(df['NumLines'], df['ParallelTime(s)'], 'r-o', label='Parallel', linewidth=2, markersize=8)
    
    plt.xlabel('Number of Log Lines', fontsize=12, fontweight='bold')
    plt.ylabel('Execution Time (seconds)', fontsize=12, fontweight='bold')
    plt.title('Performance Comparison: Serial vs Parallel', fontsize=14, fontweight='bold')
    plt.legend(fontsize=11)
    plt.grid(True, alpha=0.3)
    plt.xscale('log')
    plt.yscale('log')
    
    # Save figure
    output_dir = 'results'
    os.makedirs(output_dir, exist_ok=True)
    plt.savefig('results/serial_vs_parallel.png', dpi=300, bbox_inches='tight')
    print(f"Graph saved to results/serial_vs_parallel.png")
    
    plt.show()

def main():
    # Run benchmark if needed
    run_benchmark()
    
    # Load and plot data
    df = load_data()
    if df is not None:
        plot_performance_comparison(df)
    else:
        print("Could not load benchmark data.")

if __name__ == "__main__":
    main()

