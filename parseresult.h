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
	vector<int> v;
};

class ParseResult {
private:
	vector<string> funcspecs;
	vector<vector<int> > lastneeded;
	vector<int> realpos;
	vector<vector<instr> > instructions;
	vector<vector<int> > stoppos;
	void need(int id, int len);
public:
	bool haserror;
	vector<ClassType*> classtypes;
	map<string,Type*> types;
	map<string,FunctionDeclaration*> functions;
	stack<DeclarationInstruction*> varstack;
	map<string,DeclarationInstruction*> vars;
	Type *nullType;
	Type *voidType;
	Type *intType;
	Type *boolType;
	
	int alloc(int len);
	void copy(int from, int len, int to);
	void binaryoperate(char o, int a, int b, int c);
	void intconst(int nr, int to);
	void print(int from, int len);
	void newRef(int len, int to);
	void newArray(int unitsize, int sizepos, int to);
	void getSub(int from, int varid, int to);
	void copySub(int from, int to, int varid);
	void accessArray(int unitsize, int from, int pos, int to);
	void setArray(int unitsize, int from, int to, int pos);
	int newStop();
	void hereStop(int stop);
	void jumpIf(int cond, int stop);
	void jump(int stop);
	void call(int func, const std::vector< int >& args, int resultpos);
	void dump(char op);
	
	void output();
	
	ParseResult();
	static ParseResult *self();
};

#define ParseRes ParseResult::self()

#endif
