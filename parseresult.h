#ifndef PARSERESULT_H
#define PARSERESULT_H

#include <map>
#include <stack>
#include "type.h"
#include "instruction.h"
#include "opcodes.h"
#include "flowgraph.h"

#define INTSIZE 1
#define CHARSIZE 1
#define BOOLSIZE 1
#define POINTERSIZE 1

typedef pair<string,vector<Type*> > Funcspec;

class ParseResult {
private:
	vector<string> funcspecs;
	int varnum;
	vector<Graph*> graphs;
	Node *prevnode;
	vector<Node*> stops;
	vector<FunctionDeclaration*> funcdecs;
public:
	bool printgraphs;
	bool haserror;
	vector<ClassType*> classtypes;
	map<string,Type*> types;
	map<Funcspec,FunctionDeclaration*> functions;
	stack<DeclarationInstruction*> varstack;
	map<string,DeclarationInstruction*> vars;
	Type *nullType;
	Type *voidType;
	Type *intType;
	Type *boolType;
	Type *charType;
	
	void addFunction(string *name, FunctionDeclaration *dec);
	
	void addnode(Node *n);
	
	int alloc(int len);
	void copy(int from, int len, int to);
	void binaryoperate(OPCODE o, int a, int b, int c);
	void intconst(int nr, int to);
	void charconst(char ch, int to);
	void print(OPCODE o, int from);
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
	void call(int func, const std::vector< int >& args, int resultpos);
	void dump(OPCODE op);
	
	int output();
	
	ParseResult();
	static ParseResult *self();
};

#define ParseRes ParseResult::self()

#endif
