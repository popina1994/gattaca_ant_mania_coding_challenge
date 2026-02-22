# Instructions

## Problem Formalization and Solution

We first formalize the problem from [problem description](problem_description.txt) following instructions sent in the email and saved as [instructions](instructions_general.txt). This process is described in [thought process](thought_process.MD).
In the same document, we describe a high level solution to the problem while explaining the limitations of the choices and iteratively keep adding optimizations.

## Implementation and Testing

Further, we implement the code in [source code folder](./code/cpp) (see [program instructions](./code/cpp/README.MD)) and add system tests in [test code folder](./tests).
The implementation

## Profiling

Install python requirements.txt and then run reproducibility through [run_all](./run_all_exp.sh).
You should get plots similar to the report [report](optimization_report.pdf).
We should notice that as expected naive optimization is slower due to overhead in unnecessary unordered_map hash computations, copying thus lower cache locality of the algorithm.
Keeping adjacent nodes in a vector improves cache locality and reduces unordered_map bottlenecks, which can be seen in results of optimization 1 and optimization 2.
Optimization 3 removes the overhead of unordered_map calls by storing nodes in a vector.
Optimization 4  does not bring improvements as expected due to small size of nodes.
Optimization 5 and 6 bring improvements, but not to the level expected since the number of colonies is not that huge.
However, optimization 7 and 8 bring for larger number of ants, since we would otherwise need to iterate over containers to look for alive ants.