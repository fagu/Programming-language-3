#ifndef PARSERESULT_H
#define PARSERESULT_H

#include <map>
#include <stack>
#include <vector>
using namespace std;
#include "opcodes.h"
#include "function.h"

#define INTSIZE 1
#define CHARSIZE 1
#define BOOLSIZE 1
#define POINTERSIZE 1

class Type;
class Graph;
class Node;
class ClassType;
class Environment;
class VariableDeclaration;

class ParseResult {
private:
	vector<Graph*> graphs;
	Node *prevnode;
	vector<Node*> stops;
	vector<FunctionDeclaration*> funcdecs;
	vector<FunctionDeclaration*> funcglob;
	vector<VariableDeclaration*> vars;
	Environment *env;
	
	void addnode(Node *n);
public:
	bool printgraphs;
	bool haserror;
	
	vector<ClassType*> classtypes;
	map<string,Type*> types;
	
	Type *nullType;
	Type *voidType;
	Type *intType;
	Type *boolType;
	Type *charType;
	
	void addPrim(string name, OPCODE op, Type *resulttype);
	void addPrim(string name, OPCODE op, Type *at, string an, Type *resulttype);
	void addPrim(string name, OPCODE op, Type *at, string an, Type *bt, string bn, Type *resulttype);
	
	void addFunction(FunctionDeclaration *dec);
	void addClass(ClassType *cl);
	void addVariable(VariableDeclaration *dec);
	
	int alloc(int len);
	void copy(int from, int len, int to);
	void intconst(int nr, int to);
	void charconst(char ch, int to);
	void newRef(int len, int to);
	void newArray(int unitsize, int sizepos, int to);
	void getSub(int from, int varid, int to, int len);
	void copySub(int from, int to, int varid, int len);
	void accessArray(int unitsize, int from, int pos, int to);
	void setArray(int unitsize, int from, int to, int pos);
	int newStop();
	void hereStop(int stop);
	void jumpIf(int cond, int stop);
	void jump(int stop);
	void call(int funcnum, const std::vector< int >& args, const vector<int> &argsizes, int resultpos, int resultsize);
	void call(OPCODE op, const std::vector< int >& args, const vector<int> &argsizes, int resultpos, int resultsize);
	void getStatic(int from, int len, int to);
	void setStatic(int from, int len, int to);
	
	int output();
	
	ParseResult();
	static ParseResult *self();
};

#define ParseRes ParseResult::self()

#endif
