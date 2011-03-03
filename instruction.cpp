
#include <stdio.h>
#include "instruction.h"
#include "parseresult.h"

void BinaryOperatorInstruction::find() {
	a->find();
	b->find();
	if (a->resulttype() != ParseRes->intType || b->resulttype() != ParseRes->intType)
		printerr("Wrong type in binary operator!\n");
	pos = ParseRes->alloc(resulttype()->size());
	ParseRes->binaryoperate(op, a->pos, b->pos, pos);
}

Type* BinaryOperatorInstruction::resulttype() {
	return binaryresulttype(op);
}

void IntegerConstantInstruction::find() {
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->intconst(co, pos);
}

Type* IntegerConstantInstruction::resulttype() {
	return ParseRes->intType;
}

void NullInstruction::find() {
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->intconst(0, pos);
}

Type* NullInstruction::resulttype() {
	return ParseRes->nullType;
}

void NewInstruction::find() {
	if (!ParseRes->types.count(*name))
		printerr("Type '%s' does not exist!\n", name->c_str());
	Type *t = ParseRes->types[*name];
	if (t->style() != 'C')
		printerr("Type '%s' is not a class!\n", name->c_str());
	type = (ClassType*)t;
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->newRef(type->hashsize(), pos);
}

Type* NewInstruction::resulttype() {
	return type;
}

void PrintInstruction::find() {
	a->find();
	ParseRes->print(a->pos, a->resulttype()->size());
}

Type* PrintInstruction::resulttype() {
	return ParseRes->voidType;
}

void DeclarationInstruction::find() {
	if (ParseRes->vars.count(*name))
		printerr("Multiple definition of variable '%s'!\n", name->c_str());
	ParseRes->vars[*name] = this;
	ParseRes->varstack.push(this);
	pos = ParseRes->alloc((*type)->size());
}

Type* DeclarationInstruction::resulttype() {
	return ParseRes->voidType;
}

void SetInstruction::find() {
	a->findSet(b);
}

Type* SetInstruction::resulttype() {
	return ParseRes->voidType;
}

void VariableInstruction::find() {
	if (!ParseRes->vars[*name])
		printerr("Variable '%s' does not exist!\n", name->c_str());
	dec = ParseRes->vars[*name];
	pos = dec->pos;
}

Type* VariableInstruction::resulttype() {
	return dec->type->real();
}

void VariableInstruction::findSet(Instruction* b) {
	find();
	b->find();
	if (!b->resulttype()->convertibleTo(resulttype()))
		printerr("Types do not match!\n");
	ParseRes->copy(b->pos, b->resulttype()->size(), pos);
}

void AccessInstruction::find() {
	a->find();
	if (a->resulttype()->style() != 'C')
		printerr("Expression is not a class!\n");
	dec = ((ClassType*)a->resulttype())->var(*name);
	if (!dec)
		printerr("Class does not have a Variable called '%s'!\n", name->c_str());
	pos = ParseRes->alloc(dec->type->real()->size());
	ParseRes->getSub(a->pos, dec->pos, pos, dec->type->real()->size());
}

Type* AccessInstruction::resulttype() {
	return dec->type->real();
}

void AccessInstruction::findSet(Instruction* b) {
	a->find();
	if (a->resulttype()->style() != 'C')
		printerr("Expression is not a class!\n");
	dec = ((ClassType*)a->resulttype())->var(*name);
	if (!dec)
		printerr("Class does not have a Variable called '%s'!\n", name->c_str());
	b->find();
	if (!b->resulttype()->convertibleTo(resulttype()))
		printerr("Types do not match!\n");
	ParseRes->copySub(b->pos, a->pos, dec->pos, dec->type->real()->size());
}

void AccessArrayInstruction::find() {
	a->find();
	b->find();
	if (a->resulttype()->style() != 'A')
		printerr("Expression is not an array!\n");
	int unitsize = ((ArrayType*)a->resulttype())->contenttype->size();
	pos = ParseRes->alloc(unitsize);
	ParseRes->accessArray(unitsize, a->pos, b->pos, pos);
}

Type* AccessArrayInstruction::resulttype() {
	return ((ArrayType*)a->resulttype())->contenttype;
}

void AccessArrayInstruction::findSet(Instruction* c) {
	a->find();
	b->find();
	if (a->resulttype()->style() != 'A')
		printerr("Expression is not an array!\n");
	int unitsize = ((ArrayType*)a->resulttype())->contenttype->size();
	c->find();
	if (!c->resulttype()->convertibleTo(resulttype()))
		printerr("Types do not match!\n");
	ParseRes->setArray(unitsize, c->pos, a->pos, b->pos);
}

void IfInstruction::find() {
	cond->find();
	int elsepos = ParseRes->newStop();
	int end = ParseRes->newStop();
	ParseRes->jumpIf(cond->pos, elsepos);
	then->find();
	ParseRes->jump(end);
	ParseRes->hereStop(elsepos);
	Else->find();
	ParseRes->hereStop(end);
}

Type* IfInstruction::resulttype() {
	return ParseRes->voidType;
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

void CallInstruction::find() {
	if (!ParseRes->functions.count(*name))
		printerr("Function '%s' does not exist!\n", name->c_str());
	dec = ParseRes->functions[*name];
	if (dec->parameters->size() != arguments->size())
		printerr("Function '%s' needs exactly %d instead of %d arguments!\n", name->c_str(), (int)dec->parameters->size(), (int)arguments->size());
	vector<int> args;
	for (int i = 0; i < arguments->size(); i++) {
		(*arguments)[i]->find();
		args.push_back((*arguments)[i]->pos);
	}
	pos = ParseRes->alloc((*dec->resulttype)->size());
	ParseRes->call(dec->num, args, pos);
}

Type* CallInstruction::resulttype() {
	return dec->resulttype->real();
}

void CreateArrayInstruction::find() {
	size->find();
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->newArray(contenttype->real()->size(), size->pos, pos);
}

Type* CreateArrayInstruction::resulttype() {
	return contenttype->real()->arrayType();
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

void CompoundInstruction::find() {
	for (int i = 0; i < instructions->size(); i++) {
		(*instructions)[i]->find();
	}
}

Type* CompoundInstruction::resulttype() {
	return ParseRes->voidType;
}

void EmptyInstruction::find() {
}

Type* EmptyInstruction::resulttype() {
	return ParseRes->voidType;
}

void DumpInstruction::find() {
	ParseRes->dump(op);
}

Type* DumpInstruction::resulttype() {
	return ParseRes->voidType;
}
