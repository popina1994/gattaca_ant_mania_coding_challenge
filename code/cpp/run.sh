EXE_NAME=GATTACA

cd build
rm $EXE_NAME
rm * -rf
cmake ..
make -j

#datasets=("hiveum_map_small.txt" "hiveum_map_medium.txt")
datasets=("hiveum_map_medium.txt")
# datasets=("hiveum_map_small.txt")
#num_ants=(1 2 4 8 16 32)
num_ants=(400)

#seeds=(1 4 42)
seeds=(1)
optimization_level=(0 1 2 3 4 5 6)
# optimization_level=(0 5)
# optimization_level=(5 6)

for dataset in "${datasets[@]}"; do
    for num_ant in "${num_ants[@]}"; do
        for seed in "${seeds[@]}"; do
            for opt_level in "${optimization_level[@]}"; do
                echo ""
                echo "RUN ${dataset} ${num_ant} ${seed} ${opt_level}"
                ./$EXE_NAME ${seed} ${num_ant} ${opt_level} ../../../tests/${dataset}
            done
        done
    done
done
