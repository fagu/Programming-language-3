#ifndef PARSERESULT_H
#define PARSERESULT_H

#include <map>
#include <stack>
#include "type.h"
#include "instruction.h"

struct instr {
	char typ;
	int len;
	int a;
	int b;
	int c;
};

class ParseResult {
private:
	vector<int> lastneeded;
	vector<int> realpos;
	vector<instr> instructions;
	void need(int id, int len);
public:
	vector<ClassType*> classtypes;
	map<string,Type*> types;
	map<string,FunctionDeclaration*> functions;
	stack<DeclarationInstruction*> varstack;
	map<string,DeclarationInstruction*> vars;
	Type *voidType;
	Type *intType;
	Type *boolType;
	
	int alloc(int len);
	void copy(int from, int len, int to);
	void binaryoperate(char o, int a, int b, int c);
	void intconst(int nr, int to);
	void print(int from, int len);
	
	void output();
	
	ParseResult();
	static ParseResult *self();
};

#define ParseRes ParseResult::self()

#endif
