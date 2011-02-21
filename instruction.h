#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdio.h>
#include "type.h"

class Instruction {
public:
	virtual ~Instruction() {}
	virtual void find() = 0;
	virtual Type *resulttype() = 0;
	virtual void printRPN(FILE *file) = 0;
};

class BinaryOperatorInstruction : public Instruction {
private:
	char op;
	Instruction *a, *b;
public:
	BinaryOperatorInstruction(char _op, Instruction *_a, Instruction *_b);
	void find() { a->find(); b->find(); }
	Type* resulttype();
	void printRPN(FILE* file);
};

class IntegerConstantInstruction : public Instruction {
private:
	int co;
public:
	IntegerConstantInstruction(int _co);
	void find() {}
	Type* resulttype();
	void printRPN(FILE* file);
};

class PrintInstruction : public Instruction {
private:
	Instruction *a;
public:
	PrintInstruction(Instruction* _a);
	void find() { a->find(); }
	Type* resulttype();
	void printRPN(FILE* file);
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
	void printRPN(FILE* file);
};

class SetInstruction : public Instruction {
private:
	string *name;
	DeclarationInstruction *dec;
	Instruction *a;
public:
	SetInstruction(string *_name, Instruction *_a);
	void find();
	Type* resulttype();
	void printRPN(FILE* file);
};

class BlockInstruction : public Instruction {
public:
	vector<Instruction*> instructions;
	int varsize;
public:
	void find();
	Type* resulttype();
	void printRPN(FILE* file);
};

#endif
