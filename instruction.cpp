
#include <stdio.h>
#include "instruction.h"
#include "parseresult.h"
#include "opcodes_compiler.h"
#include "function.h"
#include "type.h"

void IntegerConstantInstruction::find(Environment* e) {
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->intconst(co, pos);
}

Type* IntegerConstantInstruction::resulttype() {
	return ParseRes->intType;
}

void CharacterConstantInstruction::find(Environment* e) {
	pos = ParseRes->alloc(ParseRes->charType->size());
	ParseRes->charconst(co, pos);
}

Type* CharacterConstantInstruction::resulttype() {
	return ParseRes->charType;
}

void StringConstantInstruction::find(Environment* e) {
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->newRef(ParseRes->charType->size()*(co->length()+1), pos);
	int charpos = ParseRes->alloc(ParseRes->charType->size());
	for (int i = 0; i < co->length(); i++) {
		ParseRes->charconst((*co)[i], charpos);
		ParseRes->copySub(charpos, pos, i*ParseRes->charType->size(), ParseRes->charType->size());
	}
	ParseRes->charconst('\0', charpos);
	ParseRes->copySub(charpos, pos, co->length()*ParseRes->charType->size(), ParseRes->charType->size());
}

Type* StringConstantInstruction::resulttype() {
	return ParseRes->charType->arrayType();
}

void NullInstruction::find(Environment* e) {
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->intconst(0, pos);
}

Type* NullInstruction::resulttype() {
	return ParseRes->nullType;
}

void NewInstruction::find(Environment* e) {
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

void DeclarationInstruction::find(Environment* e) {
	e->addVariable(this);
	pos = ParseRes->alloc((*type)->size());
}

Type* DeclarationInstruction::resulttype() {
	return ParseRes->voidType;
}

void SetInstruction::find(Environment* e) {
	a->findSet(e, b);
}

Type* SetInstruction::resulttype() {
	return ParseRes->voidType;
}

void VariableInstruction::find(Environment* e) {
	dec = e->findVariable(*name);
	pos = dec->pos;
}

Type* VariableInstruction::resulttype() {
	return dec->type->real();
}

void VariableInstruction::findSet(Environment* e, Instruction* b) {
	find(e);
	b->find(e);
	if (b->resulttype()->distance(resulttype()) == INFTY)
		printerr("Types do not match!\n");
	ParseRes->copy(b->pos, b->resulttype()->size(), pos);
}

void AccessInstruction::find(Environment* e) {
	a->find(e);
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

void AccessInstruction::findSet(Environment* e, Instruction* b) {
	a->find(e);
	if (a->resulttype()->style() != 'C')
		printerr("Expression is not a class!\n");
	dec = ((ClassType*)a->resulttype())->var(*name);
	if (!dec)
		printerr("Class does not have a Variable called '%s'!\n", name->c_str());
	b->find(e);
	if (b->resulttype()->distance(resulttype()) == INFTY)
		printerr("Types do not match!\n");
	ParseRes->copySub(b->pos, a->pos, dec->pos, dec->type->real()->size());
}

void AccessArrayInstruction::find(Environment* e) {
	a->find(e);
	b->find(e);
	if (a->resulttype()->style() != 'A')
		printerr("Expression is not an array!\n");
	int unitsize = ((ArrayType*)a->resulttype())->contenttype->size();
	pos = ParseRes->alloc(unitsize);
	ParseRes->accessArray(unitsize, a->pos, b->pos, pos);
}

Type* AccessArrayInstruction::resulttype() {
	return ((ArrayType*)a->resulttype())->contenttype;
}

void AccessArrayInstruction::findSet(Environment* e, Instruction* c) {
	a->find(e);
	b->find(e);
	if (a->resulttype()->style() != 'A')
		printerr("Expression is not an array!\n");
	int unitsize = ((ArrayType*)a->resulttype())->contenttype->size();
	c->find(e);
	if (c->resulttype()->distance(resulttype()) == INFTY)
		printerr("Types do not match!\n");
	ParseRes->setArray(unitsize, c->pos, a->pos, b->pos);
}

void IfInstruction::find(Environment* e) {
	cond->find(e);
	if (cond->resulttype()->distance(ParseRes->boolType) == INFTY)
		printerr("Types do not match!\n");
	Instruction *convcond = cond->resulttype()->convertTo(cond, ParseRes->boolType);
	int elsepos = ParseRes->newStop();
	int end = ParseRes->newStop();
	ParseRes->jumpIf(convcond->pos, elsepos);
	then->find(e);
	ParseRes->jump(end);
	ParseRes->hereStop(elsepos);
	Else->find(e);
	ParseRes->hereStop(end);
}

Type* IfInstruction::resulttype() {
	return ParseRes->voidType;
}

void WhileInstruction::find(Environment* e) {
	int start = ParseRes->newStop();
	ParseRes->hereStop(start);
	cond->find(e);
	if (cond->resulttype()->distance(ParseRes->boolType) == INFTY)
		printerr("Types do not match!\n");
	Instruction *convcond = cond->resulttype()->convertTo(cond, ParseRes->boolType);
	int end = ParseRes->newStop();
	ParseRes->jumpIf(convcond->pos, end);
	then->find(e);
	ParseRes->jump(start);
	ParseRes->hereStop(end);
}

Type* WhileInstruction::resulttype() {
	return ParseRes->voidType;
}

void CallInstruction::find(Environment* e) {
	vector<Type*> argtypes;
	for (int i = 0; i < arguments->size(); i++) {
		(*arguments)[i]->find(e);
		argtypes.push_back((*arguments)[i]->resulttype());
	}
	Environment::MESSAGE message;
	dec = e->findFunction(name, argtypes, message);
	if (message == Environment::NONEFOUND)
		printerr("Function '%s' (with the specified argument types) does not exist!\n", name->c_str());
	else if (message == Environment::MULTIPLEFOUND)
		printerr("Function '%s' (with the specified argument types) cannot be uniquely determined!\n", name->c_str());
	vector<int> args;
	for (int i = 0; i < arguments->size(); i++) {
		Instruction * arg = (*arguments)[i];
		Instruction * convarg = arg->resulttype()->convertTo(arg, (*dec->parameters)[i]->type->real());
		args.push_back(convarg->pos);
	}
	pos = ParseRes->alloc((*dec->resulttype)->size());
	ParseRes->call(dec, args, pos);
}

Type* CallInstruction::resulttype() {
	return dec->resulttype->real();
}

void ClassCallInstruction::find(Environment* e) {
	a->find(e);
	if (a->resulttype()->style() != 'C')
		printerr("Expression is not a class!\n");
	vector<Type*> argtypes;
	argtypes.push_back(a->resulttype());
	for (int i = 0; i < arguments->size(); i++) {
		(*arguments)[i]->find(e);
		argtypes.push_back((*arguments)[i]->resulttype());
	}
	Environment::MESSAGE message;
	dec = ((ClassType*)a->resulttype())->env->findFunction(name, argtypes, message);
	if (message == Environment::NONEFOUND)
		printerr("Member function '%s' (with the specified argument types) does not exist!\n", name->c_str());
	else if (message == Environment::MULTIPLEFOUND)
		printerr("Member function '%s' (with the specified argument types) cannot be uniquely determined!\n", name->c_str());
	vector<int> args;
	args.push_back(a->pos);
	for (int i = 0; i < arguments->size(); i++) {
		Instruction * arg = (*arguments)[i];
		Instruction * convarg = arg->resulttype()->convertTo(arg, (*dec->parameters)[i]->type->real());
		args.push_back(convarg->pos);
	}
	pos = ParseRes->alloc((*dec->resulttype)->size());
	ParseRes->call(dec, args, pos);
}

Type* ClassCallInstruction::resulttype() {
	return dec->resulttype->real();
}

void CreateArrayInstruction::find(Environment* e) {
	size->find(e);
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->newArray(contenttype->real()->size(), size->pos, pos);
}

Type* CreateArrayInstruction::resulttype() {
	return contenttype->real()->arrayType();
}

void BlockInstruction::find(Environment* e) {
	e->enterBlock();
	for (int i = 0; i < instructions.size(); i++) {
		instructions[i]->find(e);
	}
	e->exitBlock();
}

Type* BlockInstruction::resulttype() {
	return ParseRes->voidType;
}

void CompoundInstruction::find(Environment* e) {
	for (int i = 0; i < instructions->size(); i++) {
		(*instructions)[i]->find(e);
	}
}

Type* CompoundInstruction::resulttype() {
	return ParseRes->voidType;
}

void EmptyInstruction::find(Environment* e) {
}

Type* EmptyInstruction::resulttype() {
	return ParseRes->voidType;
}
