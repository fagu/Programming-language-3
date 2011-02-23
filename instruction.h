#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "type.h"

class Instruction {
public:
	int pos;
	virtual ~Instruction() {}
	virtual void find() = 0;
	virtual Type *resulttype() = 0;
	virtual void findSet(Instruction *b) {fprintf(stderr, "This expression cannot be assigned a value!\n");};
};

class BinaryOperatorInstruction : public Instruction {
private:
	char op;
	Instruction *a, *b;
public:
	BinaryOperatorInstruction(char _op, Instruction *_a, Instruction *_b);
	void find();
	Type* resulttype();
};

class IntegerConstantInstruction : public Instruction {
private:
	int co;
public:
	IntegerConstantInstruction(int _co);
	void find();
	Type* resulttype();
};

class NewInstruction : public Instruction {
private:
	string *name;
	ClassType *type;
public:
	NewInstruction(string *_name);
	void find();
	Type* resulttype();
};

class PrintInstruction : public Instruction {
private:
	Instruction *a;
public:
	PrintInstruction(Instruction* _a);
	void find();
	Type* resulttype();
};

class DeclarationInstruction : public Instruction {
public:
	TypePointer *type;
	string *name;
	int pos;
public:
	DeclarationInstruction(TypePointer *_type, string *_name);
	void find();
	Type* resulttype();
};

class SetInstruction : public Instruction {
private:
	Instruction *a;
	Instruction *b;
public:
	SetInstruction(Instruction *_a, Instruction *_b);
	void find();
	Type* resulttype();
};

class VariableInstruction : public Instruction {
private:
	string *name;
	DeclarationInstruction *dec;
public:
	VariableInstruction(string *_name);
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
	AccessInstruction(Instruction *_a, string *_name);
	void find();
	Type* resulttype();
	void findSet(Instruction* b);
};

class IfInstruction : public Instruction {
private:
	Instruction *cond;
	Instruction *then;
public:
	IfInstruction(Instruction *_cond, Instruction *_then);
	void find();
	Type* resulttype();
};

class BlockInstruction : public Instruction {
public:
	vector<Instruction*> instructions;
	int varsize;
public:
	void find();
	Type* resulttype();
};

#endif
