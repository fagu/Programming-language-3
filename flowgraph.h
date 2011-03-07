#ifndef FLOWGRAPH_H
#define FLOWGRAPH_H
#include <vector>
#include <queue>
#include <stack>
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
	int sub;
	int len;
	Arg(ArgType t, int v) : argtype(t), value(v), len(0) {}
	Arg(ArgType t, int v, int l) : argtype(t), value(v), len(l) {}
};

class Phi {
public:
	int varnum;
	int setsub;
	vector<int> getsubs;
	Phi(int v, int s) : varnum(v) {getsubs.resize(s);}
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
	vector<int> paridinchild;
	void searchNodes(Graph& g, Node* p);
	
	// Dominator tree
	int semi;
	Node *dom;
	vector<Node*> domsuc;
	vector<Node*> bucket;
	Node *ancestor;
	Node *label;
	Node *eval();
	void compress();
	
	// Convert to SSA form
	vector<Node*> domfront;
	int ssamaxphi;
	int ssainqueue;
	vector<Phi*> phis;
	void visitSSA(Graph& g);
	
	// Constant propagation
	int unconstgets;
	void eval(Graph& g, queue<Node*> &qu);
	
	// Dead code elimination
	void removeNode();
	
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

class Variable {
public:
	vector<Node*> setters;
	vector<vector<Node*> > getters;
	stack<int> aktsub;
	int nextsub;
	vector<int> consts;
	Variable() : nextsub(1) {aktsub.push(0);}
};

class Graph {
public:
	Node *start;
	int varnum;
	int nextstopid;
	
	vector<Node*> nodes;
	
	vector<Variable*> vars;
	
	Graph() : nextstopid(0), varnum(0) {start = new Node(VIRTUAL_START);}
	void removeOldStops();
	void buildDomTree();
	void convertToSSA();
	void constantPropagation();
	void deadCodeElimination();
	void livenessAnalysis();
	void addNewStops();
	void printGraph(FILE *fi);
};

#endif // FLOWGRAPH_H