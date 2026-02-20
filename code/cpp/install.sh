python3 -m venv ~/myenv
source ~/myenv/bin/activate

pip install gprof2dot

gprof my_program gmon.out | gprof2dot -s -o callgraph.dot
dot -Tpng callgraph.dot -o callgraph.png