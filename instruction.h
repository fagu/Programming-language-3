#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdio.h>
#include <vector>
using namespace std;
#include "location.h"
#include "opcodes.h"

class Type;
class TypePointer;
class ClassType;
class VariableDeclaration;
class BlockInstruction;
class Function;

class Instruction {
public:
	Location loc;
	int pos;
	Instruction(Location _loc) : loc(_loc) {}
	virtual ~Instruction() {}
	virtual void find() = 0;
	virtual Type *resulttype() = 0;
	virtual void findSet(Instruction *b) {fprintf(stderr, "This expression cannot be assigned a value!\n");};
};

class IntegerConstantInstruction : public Instruction {
private:
	int co;
public:
	IntegerConstantInstruction(Location _loc, int _co) : Instruction(_loc), co(_co) {}
	void find();
	Type* resulttype();
};

class CharacterConstantInstruction : public Instruction {
private:
	char co;
public:
	CharacterConstantInstruction(Location _loc, char _co) : Instruction(_loc), co(_co) {}
	void find();
	Type* resulttype();
};

class StringConstantInstruction : public Instruction {
private:
	string *co;
public:
	StringConstantInstruction(Location _loc, string *_co) : Instruction(_loc), co(_co) {}
	void find();
	Type* resulttype();
};

class NullInstruction : public Instruction {
public:
	NullInstruction(Location _loc) : Instruction(_loc) {}
	void find();
	Type* resulttype();
};

class NewInstruction : public Instruction {
private:
	string *name;
	ClassType *type;
public:
	NewInstruction(Location _loc, string *_name) : Instruction(_loc), name(_name) {}
	void find();
	Type* resulttype();
};

class DeclarationInstruction : public Instruction {
public:
	TypePointer *type;
	string *name;
	int pos;
public:
	DeclarationInstruction(Location _loc, TypePointer *_type, string *_name) : Instruction(_loc), type(_type), name(_name) {}
	void find();
	Type* resulttype();
};

class SetInstruction : public Instruction {
private:
	Instruction *a;
	Instruction *b;
public:
	SetInstruction(Location _loc, Instruction *_a, Instruction *_b) : Instruction(_loc), a(_a), b(_b) {}
	void find();
	Type* resulttype();
};

class VariableInstruction : public Instruction {
private:
	string *name;
	DeclarationInstruction *dec;
public:
	VariableInstruction(Location _loc, string *_name) : Instruction(_loc), name(_name) {}
	void find();
	Type* resulttype();
	void findSet(Instruction* b);
};

class AccessInstruction : public Instruction {
private:
	string *name;
	VariableDeclaration *dec;
	Instruction *a;
public:
	AccessInstruction(Location _loc, Instruction *_a, string *_name) : Instruction(_loc), a(_a), name(_name) {}
	void find();
	Type* resulttype();
	void findSet(Instruction* b);
};

class AccessArrayInstruction : public Instruction {
private:
	Instruction * a;
	Instruction * b;
public:
	AccessArrayInstruction(Location _loc, Instruction * _a, Instruction * _b) : Instruction(_loc), a(_a), b(_b) {}
	void find();
	Type* resulttype();
	void findSet(Instruction* b);
};

class IfInstruction : public Instruction {
private:
	Instruction *cond;
	BlockInstruction *then;
	BlockInstruction *Else;
public:
	IfInstruction(Location _loc, Instruction *_cond, BlockInstruction *_then, BlockInstruction *_else) : Instruction(_loc), cond(_cond), then(_then), Else(_else) {}
	void find();
	Type* resulttype();
};

class WhileInstruction : public Instruction {
private:
	Instruction *cond;
	BlockInstruction *then;
public:
	WhileInstruction(Location _loc, Instruction *_cond, BlockInstruction *_then) : Instruction(_loc), cond(_cond), then(_then) {}
	void find();
	Type* resulttype();
};

class CallInstruction : public Instruction {
private:
	string *name;
	Function *dec;
	vector<Instruction*> *arguments;
public:
	CallInstruction(Location _loc, string *_name, vector<Instruction*> *_arguments) : Instruction(_loc), name(_name), arguments(_arguments) {}
	void find();
	Type* resulttype();
};

class ClassCallInstruction : public Instruction {
private:
	Instruction *a;
	string *name;
	Function *dec;
	vector<Instruction*> *arguments;
public:
	ClassCallInstruction(Location _loc, Instruction *_a, string *_name, vector<Instruction*> *_arguments) : Instruction(_loc), a(_a), name(_name), arguments(_arguments) {}
	void find();
	Type* resulttype();
};

class CreateArrayInstruction : public Instruction {
private:
	TypePointer * contenttype;
	Instruction * size;
public:
	CreateArrayInstruction(Location _loc, TypePointer * _contenttype, Instruction *_size) : Instruction(_loc), contenttype(_contenttype), size(_size) {}
	void find();
	Type* resulttype();
};

class BlockInstruction : public Instruction {
public:
	vector<Instruction*> instructions;
	int varsize;
public:
	BlockInstruction(Location _loc) : Instruction(_loc) {}
	BlockInstruction(Location _loc, Instruction* _i) : Instruction(_loc) {instructions.push_back(_i);}
	void find();
	Type* resulttype();
};

class CompoundInstruction : public Instruction {
private:
	vector<Instruction*> * instructions;
public:
	CompoundInstruction(Location _loc, vector<Instruction*> *_instructions) : Instruction(_loc), instructions(_instructions) {}
	void find();
	Type* resulttype();
};

class EmptyInstruction : public Instruction {
public:
	EmptyInstruction(Location _loc) : Instruction(_loc) {}
	void find();
	Type* resulttype();
};

#endif
