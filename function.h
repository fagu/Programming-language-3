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

class Function {
public:
	string *name;
	vector<DeclarationInstruction*> * parameters;
	TypePointer * resulttype;
	Function(string *_name, vector<DeclarationInstruction*> * _parameters, TypePointer * _resulttype) : name(_name), parameters(_parameters), resulttype(_resulttype) {}
	virtual ~Function() {}
	virtual char type() = 0;
};

class FunctionDeclaration : public Function {
private:
	BlockInstruction* instructions;
public:
	Location loc;
	int num;
	FunctionDeclaration(Location _loc, string *_name, vector<DeclarationInstruction*> * _parameters, BlockInstruction * _instructions, TypePointer * _resulttype) : Function(_name, _parameters, _resulttype), loc(_loc), instructions(_instructions) {}
	~FunctionDeclaration() {}
	char type() {return 'D';}
	int find();
};

class PrimitiveFunction : public Function {
public:
	OPCODE op;
	PrimitiveFunction(string *_name, vector<DeclarationInstruction*> * _parameters, OPCODE _op, TypePointer * _resulttype) : Function(_name, _parameters, _resulttype), op(_op) {}
	~PrimitiveFunction() {}
	char type() {return 'V';}
};

class FunctionSet {
public:
	multimap<string,Function*> functions;
	void addFunction(Function *func);
	enum MESSAGE {OK, NONEFOUND, MULTIPLEFOUND};
	Function * findFunction(string *name, const vector<Type*> &argtypes, MESSAGE &message);
};

#endif // FUNCTION_H