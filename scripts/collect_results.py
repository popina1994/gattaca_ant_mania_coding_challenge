import pandas as pd
import argparse
import csv

def compute_statistics(file_path):
    # Read the CSV file
    data = pd.read_csv(file_path, sep=',', header=None, names=['ns', 'ms'])
    data['ns'] = pd.to_numeric(data['ns'], errors='coerce')
    data['ms'] = pd.to_numeric(data['ms'], errors='coerce')

    # Compute mean and standard deviation for both columns
    ns_mean = data['ns'].mean()
    ns_std = data['ns'].std()
    ms_mean = data['ms'].mean()
    ms_std = data['ms'].std()

    return ns_mean, ns_std, ms_mean, ms_std

def collect_results(results_path, seeds, ants, optimization_levels, output_file):
    results = []
    for seed in seeds:
        for ant in ants:
            for opt in optimization_levels:
                file_name = f'{results_path}/result_seed{seed}_ants{ant}_opt{opt}.csv'
                ns_mean, ns_std, ms_mean, ms_std = compute_statistics(file_name)
                results.append([seed, ant, opt, ns_mean, ns_std, ms_mean, ms_std])

    # Write results to CSV
    with open(output_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Seed', 'Ants', 'Optimization Level', 'NS Mean', 'NS Std', 'MS Mean', 'MS Std'])
        writer.writerows(results)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Collect results from CSV files.')
    parser.add_argument('results_path', type=str)
    parser.add_argument('--seeds', type=int, nargs='+', required=True)
    parser.add_argument('--ants', type=int, nargs='+', required=True)
    parser.add_argument('--opt-levels', type=int, nargs='+', required=True)
    parser.add_argument('--output', type=str, required=True, help='Output CSV file for statistics')

    args = parser.parse_args()

    collect_results(args.results_path, args.seeds, args.ants, args.opt_levels, args.output)

# Example command to run the script:
# python3 collect_results.py results --seeds 1 2 3 --ants 50 100 --opt-levels 0 2 3 --output statistics.csv