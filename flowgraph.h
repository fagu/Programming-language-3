#ifndef FLOWGRAPH_H
#define FLOWGRAPH_H
#include <vector>
#include <queue>
#include <boost/dynamic_bitset.hpp>
#include "opcodes.h"
using namespace std;
using namespace boost;

enum ArgType {
	GETARG,
	SETARG,
	INTARG
};

class Arg {
public:
	ArgType argtype;
	int value;
	int len;
	Arg(ArgType t, int v) : argtype(t), value(v), len(-1) {}
	Arg(ArgType t, int v, int l) : argtype(t), value(v), len(l) {}
};

class Graph;

class Node {
public:
	OPCODE op;
	vector<Arg*> args;
	void init(OPCODE _op);
	Node(OPCODE _op) {init(_op);}
	Node(OPCODE _op, Arg *a1) {init(_op); args.push_back(a1);}
	Node(OPCODE _op, Arg *a1, Arg *a2) {init(_op); args.push_back(a1); args.push_back(a2);}
	Node(OPCODE _op, Arg *a1, Arg *a2, Arg *a3) {init(_op); args.push_back(a1); args.push_back(a2); args.push_back(a3);}
	Node(OPCODE _op, Arg *a1, Arg *a2, Arg *a3, Arg *a4) {init(_op); args.push_back(a1); args.push_back(a2); args.push_back(a3); args.push_back(a4);}
	
	vector<Node*> suc;
	
	// Remove HERE_STOP nodes
	bool inrem;
	void removeStops();
	// DFS
	int num; // Discovery time
	vector<Node*> par;
	void searchNodes(Graph& g, Node* p);
	
	// Dominator tree
	int semi;
	Node *dom;
	vector<Node*> bucket;
	Node *ancestor;
	Node *label;
	Node *eval();
	void compress();
	
	// Live variables
	dynamic_bitset<> liveget;
	dynamic_bitset<> liveset;
	dynamic_bitset<> livein;
	dynamic_bitset<> liveout;
	bool inlivequeue;
	void initLive(Graph &g);
	bool updateLive();
	
	// Add stops for output
	int stopid;
	bool ingood, instack;
	void addGoodStops(Graph &g);
	
	// Output code
	Node* outputsuc;
	bool outputprefound;
	void print();
	
	// Visualize graph
	bool graphprinted;
	void printGraph(Graph& g, FILE* fi);
};

class Graph {
public:
	Node *start;
	int varnum;
	int nextstopid;
	
	vector<Node*> nodes;
	
	Graph() : nextstopid(0), varnum(0) {start = new Node(VIRTUAL_START);}
	void removeOldStops();
	void buildDomTree();
	void livenessAnalysis();
	void addNewStops();
	void printGraph(FILE *fi);
};

#endif // FLOWGRAPH_H