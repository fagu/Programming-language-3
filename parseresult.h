#ifndef PARSERESULT_H
#define PARSERESULT_H

#include <map>
#include <stack>
#include "type.h"
#include "instruction.h"

class ParseResult {
public:
	vector<ClassType*> classtypes;
	map<string,Type*> types;
	map<string,FunctionDeclaration*> functions;
	stack<DeclarationInstruction*> varstack;
	map<string,DeclarationInstruction*> vars;
	int firstemptypos;
	Type *voidType;
	Type *intType;
	Type *boolType;
	ParseResult();
	static ParseResult *self();
};

#define ParseRes ParseResult::self()

#endif
