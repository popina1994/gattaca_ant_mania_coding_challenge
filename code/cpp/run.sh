g++ -g -O2 -fno-omit-frame-pointer main.cpp -std=c++23 -DLOG_ON=0 -o main

./main 1 4 ../../tests/hiveum_map_small.txt
# gprof main gmon.out | gprof2dot -s -o callgraph.dot
# dot -Tpng callgraph.dot -o callgraph.png

# gprof main gmon.out > profile.txt
# gprof -q main gmon.out > profile_callgraph.txt

# ./main 1 100 ../../tests/hiveum_map_medium.txt

# gprof my_program gmon.out | gprof2dot -s -o callgraph.dot
#dot -Tpng callgraph.dot -o callgraph.png