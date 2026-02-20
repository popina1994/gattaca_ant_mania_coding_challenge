# g++ -g -O2 -fno-omit-frame-pointer main.cpp -std=c++23 -DLOG_ON=0 -o main
cd build
cmake ..
make -j

./GATTACA 1 4 ../../../tests/hiveum_map_small.txt
./GATTACA 1 100 ../../../tests/hiveum_map_medium.txt