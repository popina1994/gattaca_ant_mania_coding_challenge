import sys
import argparse
import subprocess

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run the ant simulation.')
    parser.add_argument('--seed', type=str, help='Random seed')
    parser.add_argument('--num_ants', type=int, help='Number of ants')
    parser.add_argument('--opt_level', type=str, help='Optimization level')
    parser.add_argument('--dataset', type=str, help='Dataset name')

    args = parser.parse_args()
    EXE_NAME = "GATTACA"

    output_file = f"./outputs/out_{args.seed}_{args.num_ants}_{args.opt_level}_{args.dataset}"
    command = f"./code/cpp/build/{EXE_NAME} {args.seed} {args.num_ants} {args.opt_level} tests/{args.dataset}"
    with open(output_file, 'w') as outfile:
        subprocess.run(command, shell=True, stdout=outfile)

    #  python3 ./scripts/run.py --dataset=hiveum_map_medium.txt --num_antss=2000 --seed=1 --opt_level=0