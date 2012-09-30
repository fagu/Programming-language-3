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
};

class FunctionDefinition : public Function {
public:
	string *name;
	Location loc;
	int num;
	FunctionDefinition(Location _loc, vector<DeclarationInstruction*> * _parameters, TypePointer * _resulttype) : Function(_parameters, _resulttype), loc(_loc) {name=new string("func");}
	FunctionDefinition(Location _loc, string *_name, vector<DeclarationInstruction*> * _parameters, TypePointer * _resulttype) : Function(_parameters, _resulttype), loc(_loc), name(_name) {}
	~FunctionDefinition() {}
	virtual int find(Environment *e) = 0;
	TypePointerFunction *funcType();
};

class FunctionDefinitionPrimitive : public FunctionDefinition {
private:
	OPCODE opcode;
public:
	FunctionDefinitionPrimitive(Location _loc, string* _name, OPCODE _opcode, vector< DeclarationInstruction* >* _parameters, TypePointer* _resulttype) : FunctionDefinition(_loc, _name, _parameters, _resulttype), opcode(_opcode) {}
	int find(Environment* e);
};

class FunctionDefinitionCode : public FunctionDefinition {
private:
	BlockInstruction* instructions;
public:
	Environment *env;
	FunctionDefinitionCode(Location _loc, vector< DeclarationInstruction* >* _parameters, BlockInstruction *_instructions, TypePointer* _resulttype) : FunctionDefinition(_loc, _parameters, _resulttype), instructions(_instructions), env(0) {}
	FunctionDefinitionCode(Location _loc, string *_name, vector< DeclarationInstruction* >* _parameters, BlockInstruction *_instructions, TypePointer* _resulttype) : FunctionDefinition(_loc, _name, _parameters, _resulttype), instructions(_instructions), env(0) {}
	int find(Environment* e);
};

#endif // FUNCTION_H