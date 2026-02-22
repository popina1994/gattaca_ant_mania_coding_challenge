seeds=(1 5 42)
num_ants=(1 10 100 1000 5000)
opt_levels=(0 1 2 3 4 5 6 7 8)

for seed in "${seeds[@]}"; do
    for ants in "${num_ants[@]}"; do
        for opt_level in "${opt_levels[@]}"; do
            python3 ./scripts/run.py --dataset=hiveum_map_medium.txt --num_ants=$ants --seed=$seed --opt_level=$opt_level

            # Parse the results after each experiment
            python3 ./scripts/parse_result.py \
  --input_file="./outputs/out_${seed}_${ants}_${opt_level}_hiveum_map_medium.txt" \
  --output_file="results/result_seed${seed}_ants${ants}_opt${opt_level}.csv"
        done
    done
done

# Collect results using the same arrays
python3 ./scripts/collect_results.py results \
    --seeds "${seeds[@]}" \
    --ants "${num_ants[@]}" \
    --opt-levels "${opt_levels[@]}" \
    --output statistics.csv

# Generate PDF report
python3 ./scripts/generate_table.py \
    --input_csv ./statistics.csv \
    --output_pdf ./optimization_report.pdf
