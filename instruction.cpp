
#include <stdio.h>
#include "instruction.h"
#include "parseresult.h"

BinaryOperatorInstruction::BinaryOperatorInstruction(char _op, Instruction* _a, Instruction* _b) : op(_op), a(_a), b(_b) {
}

void BinaryOperatorInstruction::find() {
	a->find();
	b->find();
	if (a->resulttype() != ParseRes->intType || b->resulttype() != ParseRes->intType)
		fprintf(stderr, "Wrong type in binary operator!");
	pos = ParseRes->alloc(resulttype()->size());
	ParseRes->binaryoperate(op, a->pos, b->pos, pos);
}

Type* BinaryOperatorInstruction::resulttype() {
	return ParseRes->intType;
}

IntegerConstantInstruction::IntegerConstantInstruction(int _co) : co(_co) {
}

void IntegerConstantInstruction::find() {
	pos = ParseRes->alloc(1);
	ParseRes->intconst(co, pos);
}

Type* IntegerConstantInstruction::resulttype() {
	return ParseRes->intType;
}

PrintInstruction::PrintInstruction(Instruction* _a) : a(_a) {
}

void PrintInstruction::find() {
	a->find();
	ParseRes->print(a->pos, a->resulttype()->size());
}

Type* PrintInstruction::resulttype() {
	return ParseRes->voidType;
}

DeclarationInstruction::DeclarationInstruction(TypePointer* _type, string* _name): type(_type), name(_name) {
}

void DeclarationInstruction::find() {
	if (ParseRes->vars.count(*name))
		fprintf(stderr, "Multiple definition of variable '%s'!\n", name->c_str());
	ParseRes->vars[*name] = this;
	ParseRes->varstack.push(this);
	pos = ParseRes->alloc((*type)->size());
}

Type* DeclarationInstruction::resulttype() {
	return ParseRes->voidType;
}

SetInstruction::SetInstruction(string* _name, Instruction* _a): name(_name), a(_a) {
}

void SetInstruction::find() {
	if (!ParseRes->vars[*name])
		fprintf(stderr, "Variable '%s' does not exist!\n", name->c_str());
	dec = ParseRes->vars[*name];
	a->find();
	if (dec->type->real() != a->resulttype())
		fprintf(stderr, "Types do not match!\n");
	ParseRes->copy(a->pos, a->resulttype()->size(), dec->pos);
}

Type* SetInstruction::resulttype() {
	return ParseRes->voidType;
}

VariableInstruction::VariableInstruction(string* _name): name(_name) {
}

void VariableInstruction::find() {
	if (!ParseRes->vars[*name])
		fprintf(stderr, "Variable '%s' does not exist!\n", name->c_str());
	dec = ParseRes->vars[*name];
	pos = dec->pos;
}

Type* VariableInstruction::resulttype() {
	return dec->type->real();
}

void BlockInstruction::find() {
	int sizebefore = ParseRes->varstack.size();
	for (int i = 0; i < instructions.size(); i++) {
		instructions[i]->find();
	}
	while(ParseRes->varstack.size() > sizebefore) {
		ParseRes->vars.erase(ParseRes->vars.find(*ParseRes->varstack.top()->name));
		ParseRes->varstack.pop();
	}
}

Type* BlockInstruction::resulttype() {
	return ParseRes->voidType;
}
