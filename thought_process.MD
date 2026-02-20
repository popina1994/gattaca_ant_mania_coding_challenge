# Thought Process

---
We first formalize the problem from [problem description](problem_description.txt) following instructions sent in the email and saved as [instructions](instructions_general.txt).

## Task Limitations

Based on the description.txt the following limitations apply to the task.

1. Limitations with regards to the colony structure.
a. Each colony is connected to other colony at most once.
b. Each colony has at most four out connections to other colonies.
c. Each out connection from a colony have at most one of the following labels: North, South, East, and West.
d. Two colonies can have the out connection with the same label to the same colony.
2. Limitations with regards to ants.
a. Each living ant is located in exactly one colony.
b. At each step, a living ant moves uniformly random out from the colony to available non-destroyed colonies or if the colony is isolated stays.
c. When two or more living ants occupy the same colony at the same step, they are destroyed, and that colony is destroyed.
d. There are no more steps if all the ants are destroyed or if each living ant went through 10000 steps.

Abstraction process.
A colony is a node in a graph, an out connection is an out edge.

## Solution Process

Here we split the task in subtasks and start with naive implementation and keep adding optimizations.

### Data Structure to Implement Graph

1. Let us start with the naive solution.
I implement a graph using an adjacency list.

Each node has a unique identifier (colony name: string-unique id: integer), and at most for out edges (hash map [string]-> node).

```cpp
unordered_map<string, Node*> htNodes;
struct Node
{
    unordered_map<string, Node*> htOutEdges;
}
```

2. Optimization 1, instead of strings, store all the nodes in one vector array and for each node preallocate an array of four out connection pointers, where instead of a hash table we would store all nodes. This would improve cache locality.
So

```cpp
struct Node
{
    string nodeName;
    pair<string, Node*>  htOutEdges[4];
};
```
vector<Node> vNodes;
3. Optimization 2. Remove out connection names from out connections to improve cache locality.


```cpp
Node
{
    string nodeName;
    Node* outEdges[4];
}
```

4. Store nodes in vectors instead of hash maps, and replace nodeName with nodeIdx, while keeping node names separately  to further improve cache locality.

```cpp
vector<Node> vNodes;
vector<string> vNodeNames;
Node
{
    Node* outEdges[4];
}
```

5. Optimization 3, use cache align each of the nodes to not cause??
6. Optimization 4, try to see what is happening if we remove nodes corresponding to destroyed colonies.

### Data structure to simulate ants

1. Let us start with the naive solution.
I implement a vector of each of N ants, with the corresponding colony positions.  Then, in each step, I iterate through all alive ants and group them by the colony. Then, for each colony where there are more than one ants, we destroy ants.
We can simulate the destruction of the living ant either by deleting the corresponding entry in the data structure storing the living ants or setting the corresponding ant entry to dead.
For all other ants we move them randomly..
We repeat this until either there are no living ants or each ant passed 10000 steps.

```cpp
struct Ant
{
    Version where colony is known only by name:
    string colony; // int colony
    Version where colony is identfied by integer
    int colony;
    bool alive;
}

vector<Ant> vAnts;
// Alive ants.
unordered_set<int>  htAnt;
```

Reasoning. We cannot have any steps skipped for the moving ants, since the ants are moving at random at network. For the ants inside the isolated colonies, we can move step by step.

2. Optimization 1. Replace ant structure with only int, and use solely the unordered_set for tracking which ants are alive/dead/isolated. This would improve the cache locality and reduce memory usage.

### Loading part

We do not count towards simulation latency the time to load and initialize the graph storage for colonies, the initialization of ants and their positions.

### Initial assumption
The biggest bottleneck will be on simulating ant movements and collision detection.

## Profiling
The following approaches for further microoptimizations:
-memory allocation/deallocation (reuse the pool)
-branching??
-caching

