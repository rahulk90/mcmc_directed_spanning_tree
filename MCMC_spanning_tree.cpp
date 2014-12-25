/* Generate an MCMC estimate of the edge appearance probabilities of a graph 
 * in the directed spanning tree polytope by sampling random spanning trees
 *
 * Author : Rahul G. Krishnan
 * Inst.  : NYU
 */
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/array.hpp>
#include <string>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp> 
#include <boost/tuple/tuple_io.hpp> 
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/graph/iteration_macros.hpp>
#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

//Parameters for the MCMC algorithm
int BURNIN = 50;
int MAXITS  = 10000;


//Defining types
typedef boost::property<boost::edge_weight_t, double> EdgeWeightProperty;
typedef boost::adjacency_list < 
    boost::vecS, boost::vecS, boost::directedS,
    boost::no_property,EdgeWeightProperty > digraph_t;
typedef boost::unordered_map<std::string,double> unordered_map;
boost::random::mt19937 rng;
#define getKey(v1,v2) std::to_string((long long unsigned int)v1)+"->"+std::to_string((long long unsigned int)v2)

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
void runTest(int n_vertices,
	std::vector<boost::tuple<int,int,double> > edgeList,
	unordered_map* edgeProb,
	std::vector<double>* root_prob)
{
    digraph_t g;
    //std::vector<double> root_prob (n_vertices);
    //unordered_map edgeProb;

    initializeGraph(edgeList,&g,edgeProb);
    for (unordered_map::iterator it = edgeProb->begin(); it != edgeProb->end(); ++it) 
        DEBUG_MSG(it->first << ", " << it->second);
    
    // boost::shared_array_property_map
    // < 
    // double, boost::property_map<digraph_t, boost::edge_weight_t> 
    // > weight(num_edges(g), get(edge_weight, g));

    //BGL_FORALL_EDGES(e, g, digraph_t) {put(weight, e, (1. + get(edge_index, g, e)) / num_edges(g));}
  	#ifdef DEBUG
    BGL_FORALL_EDGES(e, g, digraph_t) 
    {
        std::cout<<e<<std::endl;
        //put(weight, e, (1. + get(edge_index, g, e)) / num_edges(g));
    }
    #endif
    std::vector<int> predecessors (n_vertices);
    
    for(int i=0;i<n_vertices;i++)
    {
        predecessors[i]=0;
        (*root_prob)[i] = 0;
    }
    int root;
    boost::random::uniform_int_distribution<> dist(0, n_vertices-1);
    for(int i=1;i<=MAXITS;i++)
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

        //Update counts
        (*root_prob)[root]+=1;
        for(int i=0;i<n_vertices;i++)
        {
            if(predecessors[i]!=-1)
            {
                edgeProb->at(getKey(predecessors[i],i)) +=1;
            }
            else
            {
                if(root!=i)
                    std::cout<<"Error. root="<<root<<" i="<<i<<std::endl;
            }
            
        }

    }
    DEBUG_MSG("");

}

int MCMC_spanning_tree(std::string fileIN,std::string fileOUT)
{
	//Read graph structure from fileIN
	int v1,v2,n_vertices,n_edges = 0;
	std::vector< boost::tuple<int,int,double> > edgeList;
	std::string line;
	double weight;
	std::ifstream inputf (fileIN.c_str());
	if(inputf.is_open())
	{
		getline(inputf,line);
		sscanf(line.c_str(),"%d",&n_vertices);
		while(getline(inputf,line))
		{
			sscanf (line.c_str(),"%d,%d,%lf",&v1,&v2,&weight);
			//Assumes vertices start from 0...N-1
			edgeList.push_back(boost::make_tuple(v1,v2,weight));
			n_edges++;
		}
		inputf.close();
	}
	else
	{
		std::cerr<<"Input file not found. Cannot be opened\n";
		return EXIT_FAILURE;
	}

	std::vector<double> root_prob (n_vertices);
    unordered_map edgeProb;
    runTest(n_vertices,edgeList, &edgeProb, &root_prob);
    DEBUG_MSG("---RESULT---");

    
    std::ofstream outputf (fileOUT.c_str());

    //Normalize root and edge probabilities
    for (int i=0;i<n_vertices;i++)
    {
    	outputf<<(root_prob[i]/MAXITS);
    	outputf<<" ";
        DEBUG_MSG("Node "<<i<<" : " << (root_prob[i]/MAXITS));
    }
    outputf<<"\n";
    boost::tuple<int,int,double> t;
    BOOST_FOREACH(t,edgeList)
    {
    	line = getKey(boost::get<0>(t),boost::get<1>(t));
    	DEBUG_MSG(boost::get<0>(t)<<"->"<<
    	    	  boost::get<1>(t)<<" : "<<(edgeProb.at(line)/MAXITS));
    	outputf<<(edgeProb.at(line)/MAXITS);
    	outputf<<" ";
    }

	return EXIT_SUCCESS;
}

std::string PNAME = "MCMC_spanning_tree";
int main(int argc,char* argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage (* indicates optional): " << PNAME
			 << " <input file name> <output file name> <BURNIN>* <MAXIT>*\n";
		return EXIT_FAILURE;
	}
	//Modify global constants if specified
	if (argc>=4)
	{
		BURNIN = atoi(argv[3]);
		DEBUG_MSG("Modifying BURNIN to "<<BURNIN);
	}
	if (argc==5)
	{
		MAXITS = atoi(argv[4]);
		DEBUG_MSG("Modifying MAXITS to "<<MAXITS);
	}
	DEBUG_MSG("---Calling <MCMC_spanning_tree>---\nINPUT FILE: "<<argv[1]<<"\nOUTPUT FILE: "<<argv[2]);
	return MCMC_spanning_tree(argv[1],argv[2]);
	
}
