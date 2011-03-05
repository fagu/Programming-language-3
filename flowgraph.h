#ifndef FLOWGRAPH_H
#define FLOWGRAPH_H
#include <vector>
#include <queue>
#include "opcodes.h"
using namespace std;

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
	vector<Node*> suc;
	//int num;
	bool found, inrem, ingood, instack;
	int stopid;
	Node* outputsuc;
	bool outputprefound;
	void init(OPCODE _op);
	Node(OPCODE _op) {init(_op);}
	Node(OPCODE _op, Arg *a1) {init(_op); args.push_back(a1);}
	Node(OPCODE _op, Arg *a1, Arg *a2) {init(_op); args.push_back(a1); args.push_back(a2);}
	Node(OPCODE _op, Arg *a1, Arg *a2, Arg *a3) {init(_op); args.push_back(a1); args.push_back(a2); args.push_back(a3);}
	Node(OPCODE _op, Arg *a1, Arg *a2, Arg *a3, Arg *a4) {init(_op); args.push_back(a1); args.push_back(a2); args.push_back(a3); args.push_back(a4);}
	void removeStops();
	void searchNodes(Graph &g);
	void addGoodStops(Graph &g);
	void print();
};

class Graph {
public:
	Node *start;
	int nextstopid;
	vector<Node*> nodes;
	//int nextnum;
	
	Graph() : nextstopid(0) {start = new Node(VIRTUAL_START);}
	void addNewStops();
};

#endif // FLOWGRAPH_H