
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
	pos = ParseRes->firstemptypos;
	ParseRes->firstemptypos += resulttype()->size();
}

Type* BinaryOperatorInstruction::resulttype() {
	return ParseRes->intType;
}

void BinaryOperatorInstruction::printRPN(FILE* file) {
	a->printRPN(file);
	b->printRPN(file);
	fprintf(file, "A%d|%c%d;%d;%d|", resulttype()->size(), op, a->pos, b->pos, pos);
}

void BinaryOperatorInstruction::printRPNclear(FILE* file) {
	fprintf(file, "A%d|", -resulttype()->size());
	a->printRPNclear(file);
	b->printRPNclear(file);
}

IntegerConstantInstruction::IntegerConstantInstruction(int _co) : co(_co) {
}

void IntegerConstantInstruction::find() {
	pos = ParseRes->firstemptypos;
	ParseRes->firstemptypos += resulttype()->size();
}

Type* IntegerConstantInstruction::resulttype() {
	return ParseRes->intType;
}

void IntegerConstantInstruction::printRPN(FILE* file) {
	fprintf(file, "A%d|I%d;%d|", resulttype()->size(), co, pos);
}

void IntegerConstantInstruction::printRPNclear(FILE* file) {
	fprintf(file, "A%d|", -resulttype()->size());
}

PrintInstruction::PrintInstruction(Instruction* _a) : a(_a) {
}

void PrintInstruction::find() {
	a->find();
	//if (a->resulttype() != ParseRes->intType)
	//	fprintf(stderr, "Wrong type in print function!");
}

Type* PrintInstruction::resulttype() {
	return ParseRes->voidType;
}

void PrintInstruction::printRPN(FILE* file) {
	a->printRPN(file);
	fprintf(file, "P%d;%d|", a->pos, a->resulttype()->size());
}

DeclarationInstruction::DeclarationInstruction(TypePointer* _type, string* _name): type(_type), name(_name) {
}

void DeclarationInstruction::find() {
	if (ParseRes->vars.count(*name))
		fprintf(stderr, "Multiple definition of variable '%s'!\n", name->c_str());
	ParseRes->vars[*name] = this;
	ParseRes->varstack.push(this);
	pos = ParseRes->firstemptypos;
	ParseRes->firstemptypos += (*type)->size();
}

Type* DeclarationInstruction::resulttype() {
	return ParseRes->voidType;
}

void DeclarationInstruction::printRPN(FILE* file) {
	fprintf(file, "A%d|", (*type)->size());
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
}

Type* SetInstruction::resulttype() {
	return ParseRes->voidType;
}

void SetInstruction::printRPN(FILE* file) {
	a->printRPN(file);
	fprintf(file, "C%d;%d;%d|", a->pos, a->resulttype()->size(), dec->pos);
	a->printRPNclear(file);
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

void VariableInstruction::printRPN(FILE* file) {
}

void VariableInstruction::printRPNclear(FILE* file) {
}

void BlockInstruction::find() {
	int sizebefore = ParseRes->varstack.size();
	for (int i = 0; i < instructions.size(); i++) {
		instructions[i]->find();
	}
	varsize = ParseRes->firstemptypos;
	while(ParseRes->varstack.size() > sizebefore) {
		ParseRes->vars.erase(ParseRes->vars.find(*ParseRes->varstack.top()->name));
		ParseRes->firstemptypos -= (*ParseRes->varstack.top()->type)->size();
		ParseRes->varstack.pop();
	}
	varsize -= ParseRes->firstemptypos;
}

Type* BlockInstruction::resulttype() {
	return ParseRes->voidType;
}

void BlockInstruction::printRPN(FILE* file) {
	for (int i = 0; i < instructions.size(); i++) {
		instructions[i]->printRPN(file);
		instructions[i]->printRPNclear(file);
		fprintf(file, "\n");
	}
	fprintf(file, "A%d|\n", -varsize);
}
