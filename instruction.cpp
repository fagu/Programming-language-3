#include "instruction.h"
#include "parseresult.h"

BinaryOperatorInstruction::BinaryOperatorInstruction(char _op, Instruction* _a, Instruction* _b) : op(_op), a(_a), b(_b) {
	if (a->resulttype() != ParseRes->intType || b->resulttype() != ParseRes->intType)
		fprintf(stderr, "Wrong type in binary operator!");
}

Type* BinaryOperatorInstruction::resulttype() {
	return ParseRes->intType;
}

void BinaryOperatorInstruction::printRPN(FILE* file) {
	a->printRPN(file);
	b->printRPN(file);
	fprintf(file, "B%c|", op);
}

IntegerConstantInstruction::IntegerConstantInstruction(int _co) : co(_co) {
}

Type* IntegerConstantInstruction::resulttype() {
	return ParseRes->intType;
}

void IntegerConstantInstruction::printRPN(FILE* file) {
	fprintf(file, "C%d|", co);
}

PrintInstruction::PrintInstruction(Instruction* _a) : a(_a) {
	if (a->resulttype() != ParseRes->intType)
		fprintf(stderr, "Wrong type in print function!");
}

Type* PrintInstruction::resulttype() {
	return ParseRes->voidType;
}

void PrintInstruction::printRPN(FILE* file) {
	a->printRPN(file);
	fprintf(file, "P|");
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
	fprintf(file, "D%d|", (*type)->size());
}

SetInstruction::SetInstruction(string* _name, Instruction* _a): name(_name), a(_a) {
}

void SetInstruction::find() {
	if (!ParseRes->vars[*name])
		fprintf(stderr, "Variable '%s' does not exist!\n", name->c_str());
	dec = ParseRes->vars[*name];
}

Type* SetInstruction::resulttype() {
	return ParseRes->voidType;
}

void SetInstruction::printRPN(FILE* file) {
	a->printRPN(file);
	fprintf(file, "S%d|", dec->pos);
}

void BlockInstruction::find() {
	int sizebefore = ParseRes->varstack.size();
	for (int i = 0; i < instructions.size(); i++) {
		instructions[i]->find();
	}
	varsize = ParseRes->firstemptypos;
	while(ParseRes->varstack.size() > sizebefore) {
		ParseRes->vars.erase(ParseRes->vars.find(*ParseRes->varstack.top()->name));
		ParseRes->firstemptypos = ParseRes->varstack.top()->pos;
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
		fprintf(file, "\n");
	}
	fprintf(file, "E%d|\n", varsize);
}
