# Instructions
Program is compiled, and the experiments are being ran using
```bash
bash run.sh
```

# Reproducibility in run.sh

Logging steps can be turned on as requested in instructions by setting LOG_ON variable in CMakeLists.txt to true.
System tests can be changed by specifying additional system tests in datasets array in run.sh.
The number of ants can be changed by specifying additional num_ants options in num_ants array.
The seeds used for generation of ants positions and their movement can be specified in the seeds array.
The different optimization levels can be selected using from 0 to 8, where 0 presents the naive implementation of the code.

# main.cpp

Each optimizations represents a unique combination of graph representation, colony state management, and node optimization strategy.

0: Naive implementation. Graph is represented using adjacency list map where adjacent nodes are stored inside an unordered_map: direction name: node pointer. We use vector for ant based positions.

1: Graph is represented using adjacency list map where adjacent nodes are stored inside a vector of pairs: direction name: node pointer.  We use vector for ant based positions.

2: Graph is represented using an adjacency list map where adjacent nodes are stored inside an array of node pointers. We use vector for ant based positions.

3: Graph is represented using adjacency list vector where each colony name is stored separately from nodes nodes.
We store adjacent nodes inside an array of node pointers.
We use vector for ant based positions.

4: Graph is represented using adjacency list vector where each colony name is stored separately from nodes nodes.
We store adjacent nodes inside an array of node pointers.
We use vector for ant based positions.
All node and ant classes are aligned for cache size.

5: Graph is represented using adjacency list map where each colony name is stored separately from nodes nodes, but we use colony unique integer identifiers for map keys and we use map to track alive colonies.
We store adjacent nodes inside an array of node pointers.
We use vector for ant based positions.

6: Graph is represented using adjacency list map where each colony name is stored separately from nodes nodes, but we use colony unique integer identifiers for map keys and we use map to track alive colonies.
All node and ant classes are aligned for cache size.
We store adjacent nodes inside an array of node pointers.
We use vector for ant based positions.

7: Graph is represented using adjacency list map where each colony name is stored separately from nodes nodes, but we use colony unique integer identifiers for map keys.
We store adjacent nodes inside an array of node pointers.
We use unordered_map to track alive ant based positions.

8: Graph is represented using adjacency list map where each colony name is stored separately from nodes nodes, but we use colony unique integer identifiers for map keys.
All node and ant classes are aligned for cache size.
We store adjacent nodes inside an array of node pointers.
We use unordered_map to track alive ant based positions.

# Program arguments

* @param seed Random number generator seed for experiment reproducibility
* @param optimizationLevel An optimization level
* param numAnts Number of ants to simulate in the colony
* param colonyFileName Path to the colony configuration file