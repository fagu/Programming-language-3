
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

NewInstruction::NewInstruction(string* _name) : name(_name) {
}

void NewInstruction::find() {
	if (!ParseRes->types.count(*name))
		fprintf(stderr, "Type '%s' does not exist!\n", name->c_str());
	Type *t = ParseRes->types[*name];
	if (t->style() != 'C')
		fprintf(stderr, "Type '%s' is not a class!\n", name->c_str());
	type = (ClassType*)t;
	pos = ParseRes->alloc(1);
	ParseRes->newRef(type->hashsize(), pos);
}

Type* NewInstruction::resulttype() {
	return type;
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

DeclarationInstruction::DeclarationInstruction(TypePointer* _type, string* _name) : type(_type), name(_name) {
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

SetInstruction::SetInstruction(Instruction* _a, Instruction* _b): a(_a), b(_b) {
}

void SetInstruction::find() {
	a->findSet(b);
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

void VariableInstruction::findSet(Instruction* b) {
	find();
	b->find();
	if (resulttype() != b->resulttype())
		fprintf(stderr, "Types do not match!\n");
	ParseRes->copy(b->pos, b->resulttype()->size(), pos);
}

AccessInstruction::AccessInstruction(Instruction* _a, string* _name): a(_a), name(_name) {
}

void AccessInstruction::find() {
	a->find();
	if (a->resulttype()->style() != 'C')
		fprintf(stderr, "Expression is not a class!\n");
	dec = ((ClassType*)a->resulttype())->var(*name);
	if (!dec)
		fprintf(stderr, "Class does not have a Variable called '%s'!\n", name->c_str());
	pos = ParseRes->alloc(1);
	ParseRes->getSub(a->pos, dec->pos, pos);
}

Type* AccessInstruction::resulttype() {
	return dec->type->real();
}

void AccessInstruction::findSet(Instruction* b) {
	a->find();
	if (a->resulttype()->style() != 'C')
		fprintf(stderr, "Expression is not a class!\n");
	dec = ((ClassType*)a->resulttype())->var(*name);
	if (!dec)
		fprintf(stderr, "Class does not have a Variable called '%s'!\n", name->c_str());
	b->find();
	if (resulttype() != b->resulttype())
		fprintf(stderr, "Types do not match!\n");
	ParseRes->copySub(b->pos, a->pos, dec->pos);
}

IfInstruction::IfInstruction(Instruction* _cond, Instruction* _then) : cond(_cond), then(_then) {
}

void IfInstruction::find() {
	cond->find();
	int end = ParseRes->newStop();
	ParseRes->jumpIf(cond->pos, end);
	then->find();
	ParseRes->hereStop(end);
}

Type* IfInstruction::resulttype() {
	return ParseRes->voidType;
}

WhileInstruction::WhileInstruction(Instruction* _cond, Instruction* _then) : cond(_cond), then(_then) {
}

void WhileInstruction::find() {
	int start = ParseRes->newStop();
	ParseRes->hereStop(start);
	cond->find();
	int end = ParseRes->newStop();
	ParseRes->jumpIf(cond->pos, end);
	then->find();
	ParseRes->jump(start);
	ParseRes->hereStop(end);
}

Type* WhileInstruction::resulttype() {
	return ParseRes->voidType;
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
