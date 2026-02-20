g++ -g -O2 -fno-omit-frame-pointer main.cpp -std=c++23 -DLOG_ON=0 -o main

./main 1 4 ../../tests/hiveum_map_small.txt
./main 1 100 ../../tests/hiveum_map_medium.txt
