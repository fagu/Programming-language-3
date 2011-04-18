#ifndef FUNCTION_H
#define FUNCTION_H

#include <map>
using namespace std;
#include "location.h"
#include "opcodes.h"

class Type;
class TypePointer;
class DeclarationInstruction;
class BlockInstruction;
class Environment;
class TypePointerFunction;

class Function {
public:
	vector<DeclarationInstruction*> * parameters;
	TypePointer * resulttype;
	Function(vector<DeclarationInstruction*> * _parameters, TypePointer * _resulttype) : parameters(_parameters), resulttype(_resulttype) {}
	virtual ~Function() {}
	virtual char type() = 0;
};

class FunctionDefinition : public Function {
private:
	BlockInstruction* instructions;
public:
	string *name;
	Location loc;
	Environment *env;
	int num;
	FunctionDefinition(Location _loc, vector<DeclarationInstruction*> * _parameters, BlockInstruction * _instructions, TypePointer * _resulttype) : Function(_parameters, _resulttype), loc(_loc), instructions(_instructions), env(0) {name=new string("func");}
	FunctionDefinition(Location _loc, string *_name, vector<DeclarationInstruction*> * _parameters, BlockInstruction * _instructions, TypePointer * _resulttype) : Function(_parameters, _resulttype), loc(_loc), name(_name), instructions(_instructions), env(0) {}
	~FunctionDefinition() {}
	char type() {return 'D';}
	int find(Environment *e);
	TypePointerFunction *funcType();
};

#endif // FUNCTION_H