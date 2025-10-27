#!/usr/bin/env python3
"""
Plot speedup graph showing how much faster parallel is compared to serial
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import subprocess
import os

def load_data():
    """Load benchmark data from CSV"""
    try:
        df = pd.read_csv('results/benchmark_data.csv')
        return df
    except FileNotFoundError:
        print("Error: benchmark_data.csv not found. Please run the benchmark first.")
        return None

def plot_speedup(df):
    """Plot speedup graph"""
    plt.figure(figsize=(12, 7))
    
    # Plot actual speedup
    plt.plot(df['NumLines'], df['Speedup'], 'g-o', label='Measured Speedup', 
             linewidth=2.5, markersize=10, zorder=3)
    
    # Plot ideal speedup line (assuming 4 threads)
    ideal_speedup = [4.0] * len(df['NumLines'])
    plt.axhline(y=4, color='r', linestyle='--', linewidth=2, 
                label='Ideal Speedup (4 threads)', zorder=1)
    
    # Fill area between lines
    plt.fill_between(df['NumLines'], df['Speedup'], ideal_speedup, 
                     alpha=0.2, color='orange', label='Overhead')
    
    plt.xlabel('Number of Log Lines', fontsize=12, fontweight='bold')
    plt.ylabel('Speedup (Serial Time / Parallel Time)', fontsize=12, fontweight='bold')
    plt.title('Parallel Speedup Analysis', fontsize=14, fontweight='bold')
    plt.legend(fontsize=11)
    plt.grid(True, alpha=0.3)
    plt.xscale('log')
    
    # Add text annotations
    for i, row in df.iterrows():
        plt.annotate(f'{row["Speedup"]:.2f}x', 
                    (row['NumLines'], row['Speedup']),
                    textcoords="offset points", xytext=(0,15), ha='center',
                    fontsize=9, color='darkgreen', fontweight='bold')
    
    # Save figure
    output_dir = 'results'
    os.makedirs(output_dir, exist_ok=True)
    plt.savefig('results/speedup_analysis.png', dpi=300, bbox_inches='tight')
    print(f"Graph saved to results/speedup_analysis.png")
    
    plt.show()

def main():
    import os
    # Change to project directory for relative paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.join(script_dir, '..')
    os.chdir(project_dir)
    
    df = load_data()
    if df is not None:
        plot_speedup(df)
    else:
        print("Could not load benchmark data.")

if __name__ == "__main__":
    main()

