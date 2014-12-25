#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/graph/named_function_params.hpp>
#include <iostream>
#include <boost/array.hpp>
#include <string>
#include <boost/foreach.hpp>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp> 
#include <boost/tuple/tuple_io.hpp> 
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

//Defining types
typedef boost::property<boost::edge_weight_t, double> EdgeWeightProperty;
typedef boost::adjacency_list < 
    boost::vecS, boost::vecS, boost::directedS,
    boost::no_property,EdgeWeightProperty > digraph_t;

// typedef boost::property<boost::edge_index_t, int> EdgeWeightProperty;
// typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::directedS,
// boost::no_property,boost::property< boost::edge_index_t, std::size_t > > digraph_t;


typedef boost::unordered_map<std::string,double> unordered_map;
boost::random::mt19937 rng;
int MAXIT = 10000;

/*
Macro to return "v1->v2"
(Used to avoid implementing a hash function for tuples)
*/
#define getKey(v1,v2) std::to_string((long long unsigned int)v1)+"->"+std::to_string((long long unsigned int)v2)

/*
Initialize a graph using the edge list 
*/
void initializeGraph(std::vector<boost::tuple<int,int,double> > edgeList, 
                    digraph_t* g,
                    unordered_map* map) 
{   
    int v1,v2;
    double wt;
    boost::tuple<int,int,double> t;
    BOOST_FOREACH ( t, edgeList)
    {
        v1 = boost::get<0>(t);
        v2 = boost::get<1>(t);
        wt = boost::get<2>(t);
        DEBUG_MSG("Loading: "<<v1<<"->"<<v2<<" : "<<wt);
        add_edge(v1,v2,wt,*g);
        map->insert(unordered_map::value_type(getKey(v1,v2),0));
    }
}


/*
Given an edgeList, run the test case
*/
void runTest(int n_vertices,std::vector<boost::tuple<int,int,double> > edgeList)
{
    digraph_t g;

    unordered_map edgeProb;
    initializeGraph(edgeList,&g,&edgeProb);
    for (unordered_map::iterator it = edgeProb.begin(); it != edgeProb.end(); ++it) 
        DEBUG_MSG(it->first << ", " << it->second);
    
    // boost::shared_array_property_map
    // < 
    // double, boost::property_map<digraph_t, boost::edge_weight_t> 
    // > weight(num_edges(g), get(edge_weight, g));

    //BGL_FORALL_EDGES(e, g, digraph_t) {put(weight, e, (1. + get(edge_index, g, e)) / num_edges(g));}
  
    BGL_FORALL_EDGES(e, g, digraph_t) 
    {
        std::cout<<e<<std::endl;
        //put(weight, e, (1. + get(edge_index, g, e)) / num_edges(g));
    }

    std::vector<int> predecessors (n_vertices);
    std::vector<double> root_prob (n_vertices);
    for(int i=0;i<n_vertices;i++)
    {
        predecessors[i]=0;
        root_prob[i] = 0;
    }
    int root;
    boost::random::uniform_int_distribution<> dist(0, n_vertices-1);
    for(int i=1;i<=MAXIT;i++)
    {
        //Sample root uniformly 
        root = dist(rng);
        #ifdef DEBUG_L2 //Since the DEBUG_MSG macro prints newline
            std::cout<<i<<"|"<<root<<"|,"<<std::flush;
        #endif
        boost::random_spanning_tree(g,rng,
            boost::predecessor_map(
                boost::make_iterator_property_map(
                    predecessors.begin(), get(boost::vertex_index, g))).
            root_vertex(root));

        // boost::random_spanning_tree(g,rng,(boost::graph_traits<digraph_t>::vertex_descriptor)root,
        //     boost::predecessor_map(
        //         boost::make_iterator_property_map(
        //             predecessors.begin(), get(boost::vertex_index, g))));
        
        //Update counts
        root_prob[root]+=1;
        for(int i=0;i<n_vertices;i++)
        {
            if(predecessors[i]!=-1)
            {
                edgeProb.at(getKey(predecessors[i],i)) +=1;
            }
            else
            {
                if(root!=i)
                    std::cout<<"Error. root="<<root<<" i="<<i<<std::endl;
            }
            
        }

    }
    DEBUG_MSG("");

    std::cout<<"---RESULT---"<<std::endl;
    //Normalize root and edge probabilities
    for (int i=0;i<n_vertices;i++)
    {
        root_prob[i]/=MAXIT;
        std::cout<<"Node "<<i<<" : " << root_prob[i] << std::endl;
    }
    for (unordered_map::iterator it = edgeProb.begin(); it != edgeProb.end(); ++it) 
    {
        std::cout << it->first << ": " << (it->second/MAXIT) << std::endl; 
    } 
}

/*
4 node grid

Expected Result (Uniform) : The edges in grid should all have 1/4 probabilities, the edge
moving outside should have 
*/
void tc1()
{
    int n_vertices = 4;
    std::cout<<"----------- 4 Node Grid (Uniform) ------------"<<std::endl;
    std::vector<boost::tuple<int,int,double> > edgeList = 
    {
        boost::make_tuple(0,1,10), 
        boost::make_tuple(1,0,10),
        boost::make_tuple(3,1,10), 
        boost::make_tuple(1,3,10),
        boost::make_tuple(2,3,10), 
        boost::make_tuple(3,2,10),
        boost::make_tuple(0,2,10), 
        boost::make_tuple(2,0,10)
    };
    runTest(n_vertices,edgeList);
    std::cout<<"----------- Done (Check Results Visually) ------------"<<std::endl;
}


void tc2()
{
    int n_vertices = 4;
    std::cout<<"----------- 4 Node Grid (Non Uniform) ------------"<<std::endl;
    std::cout<<"----------- 0-2-3-1 has highest probability ------------"<<std::endl;
    std::cout<<"----------- 2-0-1-3 has second highest probability ------------"<<std::endl;
    std::vector<boost::tuple<int,int,double> > edgeList = 
    {
        boost::make_tuple(0,1,50), 
        boost::make_tuple(1,0,10),
        boost::make_tuple(3,1,100), 
        boost::make_tuple(1,3,50),
        boost::make_tuple(2,3,100), 
        boost::make_tuple(3,2,10),
        boost::make_tuple(0,2,100), 
        boost::make_tuple(2,0,50)
    };
    runTest(n_vertices,edgeList);
    std::cout<<"----------- Done (Check Results Visually) ------------"<<std::endl;
}



/*
4 node fully connected graph

Expected Result (Uniform) : The edges in grid should all have 1/12 probabilities, the edge
moving outside should have 

*/
void tc3()
{
    int n_vertices = 4;
    std::cout<<"----------- 4 Node Fully Connected (Uniform) ------------"<<std::endl;
    std::vector<boost::tuple<int,int,double> > edgeList = 
    {
        boost::make_tuple(0,1,10), 
        boost::make_tuple(1,0,10),
        boost::make_tuple(3,1,10), 
        boost::make_tuple(1,3,10),
        boost::make_tuple(2,3,10), 
        boost::make_tuple(3,2,10),
        boost::make_tuple(0,2,10), 
        boost::make_tuple(2,0,10),
        boost::make_tuple(0,3,10), 
        boost::make_tuple(3,0,10),
        boost::make_tuple(1,2,10), 
        boost::make_tuple(2,1,10)
    };
    runTest(n_vertices,edgeList);
    std::cout<<"----------- Done (Check Results Visually) ------------"<<std::endl;

}


/*
Graph with a single edge that appears in all 
undirected spanning trees. (2x2 grid graph + 1 extra outgrid edge)

Expected Result (Uniform) : The edges in grid should all have equal probabilities, 
the edges moving outside should have probability 0.5 for either direction
*/
void tc4()
{
    int n_vertices = 5;
    std::cout<<"----------- 4 Node Grid (Uniform) +1 ------------"<<std::endl;
    std::vector<boost::tuple<int,int,double> > edgeList = 
    {
        boost::make_tuple(0,1,10), 
        boost::make_tuple(1,0,10),
        boost::make_tuple(3,1,10), 
        boost::make_tuple(1,3,10),
        boost::make_tuple(2,3,10), 
        boost::make_tuple(3,2,10),
        boost::make_tuple(0,2,10), 
        boost::make_tuple(2,0,10),
        boost::make_tuple(0,4,10), 
        boost::make_tuple(4,0,10)
    };
    runTest(n_vertices,edgeList);
    std::cout<<"----------- Done (Check Results Visually) ------------"<<std::endl;

}

int main()
{
    tc1();
    tc2();
    tc3();
    tc4();
    return EXIT_SUCCESS;
}

/*

    digraph_t g;
    add_edge(0, 1,10, g);
    add_edge(0, 2,10, g);
    add_edge(1, 3,10, g);
    add_edge(0, 4,10, g);
    boost::array<int, 5> predecessors;

    add_edge(1, 0,10, g);
    add_edge(2, 0,10, g);
    add_edge(3, 1,10, g);
    add_edge(4, 0,10, g);

    boost::random_spanning_tree(g,rng,
    boost::predecessor_map(predecessors.begin()).root_vertex(0));
    std::cout<<"Spanning Tree"<<std::endl;
    int p = 0;
    while (p != 5)
    {
    std::cout << predecessors[p++] << '\n';
    //p = predecessors[p];
    }
    // You should expect to see 0 1 3 2 4
    return EXIT_SUCCESS;
*/

