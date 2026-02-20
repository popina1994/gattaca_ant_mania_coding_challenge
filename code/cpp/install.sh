python3 -m venv ~/myenv
source ~/myenv/bin/activate

pip install gprof2dot
sudo apt install graphviz
# https://www.amd.com/en/developer/uprof/uprof-eula/uprof-5-2-eula.html?filename=amduprof_5.2-606_amd64.deb
#gprof my_program gmon.out | gprof2dot -s -o callgraph.dot
#dot -Tpng callgraph.dot -o callgraph.png

# AMDuProfCLI profile --config hotspots --call-graph-depth 128  -o ./output_dir ./main 1 4 ../../tests/hiveum_map_small.txt
