mcmc_directed_spanning_tree
===========================

Author: Rahul G. Krishnan
Institution : NYU

This code takes as input, a file describing a graph and samples
the edge appearance probabilities from the uniform distribution over spanning
trees. The result is an MCMC sample of the edge appearance probabilities from the directed spanning tree polytope.

This code uses the Boost Graph Library and requires compilation with the the -std=c++0x flag.
Modify the Makefile as desired.

random_spanning_tree_test.cpp contains test cases for a few simple graphs.

Usage : ./MCMC_spanning_tree <input file> <output file> |Optional: BURNIN| |Optional: MAXITS|

Input file format
----------------
N
src1,dest1,weight1
src2,dest2,weight2
..
srcE,destE,weightE

where N is the number of vertices, src1 and dest1 are the vertex descriptors and weight1 is the weight associated
with the edge. src*,dest* are all expected to be in [0,N-1] and weight* are expected as positive real numbers. 

See tc1.tc for an example of a test case
(Issue: Weights ignored)

Output file format
-----------------
p_0,p_1,....,p_V
e_0,e_1.....,e_E

p_i is the probability that the ith node is the root of the directed spanning tree
e_0 is the probability that the edge'0' is in a directed spanning tree where the indices
[0....E] correspond to edges in the order that they were initially supplied in the input
file.

Known Issue
----------
Currently, the weights on the edges are ignored and therefore spanning trees are sampled
from the uniform distribution.
