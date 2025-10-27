#!/usr/bin/env python3
"""
Plot efficiency graph showing how efficiently parallel threads are being utilized
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

def plot_efficiency(df):
    """Plot efficiency graph"""
    plt.figure(figsize=(12, 7))
    
    # Plot efficiency
    plt.plot(df['NumLines'], df['Efficiency(%)'], 'b-o', label='Efficiency', 
             linewidth=2.5, markersize=10, color='navy')
    
    # Plot 100% efficiency line (ideal)
    ideal_efficiency = [100.0] * len(df['NumLines'])
    plt.axhline(y=100, color='r', linestyle='--', linewidth=2, 
                label='Ideal Efficiency (100%)', zorder=1)
    
    # Plot 50% efficiency line
    plt.axhline(y=50, color='orange', linestyle='--', linewidth=1.5, 
                alpha=0.5, label='Minimum Efficient (50%)', zorder=1)
    
    # Fill area
    plt.fill_between(df['NumLines'], df['Efficiency(%)'], 0, 
                     alpha=0.3, color='lightblue', label='Efficiency Area')
    
    plt.xlabel('Number of Log Lines', fontsize=12, fontweight='bold')
    plt.ylabel('Efficiency (%)', fontsize=12, fontweight='bold')
    plt.title('Parallel Efficiency Analysis', fontsize=14, fontweight='bold')
    plt.legend(fontsize=11, loc='best')
    plt.grid(True, alpha=0.3)
    plt.xscale('log')
    plt.ylim(0, 110)
    
    # Add text annotations
    for i, row in df.iterrows():
        plt.annotate(f'{row["Efficiency(%)"]:.1f}%', 
                    (row['NumLines'], row['Efficiency(%)']),
                    textcoords="offset points", xytext=(0,15), ha='center',
                    fontsize=9, color='darkblue', fontweight='bold')
    
    # Save figure
    output_dir = 'results'
    os.makedirs(output_dir, exist_ok=True)
    plt.savefig('results/efficiency_analysis.png', dpi=300, bbox_inches='tight')
    print(f"Graph saved to results/efficiency_analysis.png")
    
    plt.show()

def main():
    import os
    # Change to project directory for relative paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.join(script_dir, '..')
    os.chdir(project_dir)
    
    df = load_data()
    if df is not None:
        plot_efficiency(df)
    else:
        print("Could not load benchmark data.")

if __name__ == "__main__":
    main()

