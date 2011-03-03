
#include <stdio.h>
#include "instruction.h"
#include "parseresult.h"

void BinaryOperatorInstruction::find() {
	a->find();
	b->find();
	if (a->resulttype()->distance(binaryinputtype(op)) == INFTY && b->resulttype()->distance(binaryinputtype(op)) == INFTY)
		printerr("Wrong type in binary operator!\n");
	Instruction *conva = a->resulttype()->convertTo(a, binaryinputtype(op));
	Instruction *convb = b->resulttype()->convertTo(b, binaryinputtype(op));
	pos = ParseRes->alloc(resulttype()->size());
	ParseRes->binaryoperate(op, conva->pos, convb->pos, pos);
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

void CharacterConstantInstruction::find() {
	pos = ParseRes->alloc(ParseRes->charType->size());
	ParseRes->charconst(co, pos);
}

Type* CharacterConstantInstruction::resulttype() {
	return ParseRes->charType;
}

void StringConstantInstruction::find() {
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
	if (op == PRINT_INT)
		if (a->resulttype()->distance(ParseRes->intType) == INFTY)
			printerr("Types do not match!\n");
	if (op == PRINT_CHAR)
		if (a->resulttype()->distance(ParseRes->charType) == INFTY)
			printerr("Types do not match!\n");
	ParseRes->print(op, a->pos);
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
	if (b->resulttype()->distance(resulttype()) == INFTY)
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
	if (b->resulttype()->distance(resulttype()) == INFTY)
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
	if (c->resulttype()->distance(resulttype()) == INFTY)
		printerr("Types do not match!\n");
	ParseRes->setArray(unitsize, c->pos, a->pos, b->pos);
}

void IfInstruction::find() {
	cond->find();
	if (cond->resulttype()->distance(ParseRes->boolType) == INFTY)
		printerr("Types do not match!\n");
	Instruction *convcond = cond->resulttype()->convertTo(cond, ParseRes->boolType);
	int elsepos = ParseRes->newStop();
	int end = ParseRes->newStop();
	ParseRes->jumpIf(convcond->pos, elsepos);
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
	if (cond->resulttype()->distance(ParseRes->boolType) == INFTY)
		printerr("Types do not match!\n");
	Instruction *convcond = cond->resulttype()->convertTo(cond, ParseRes->boolType);
	int end = ParseRes->newStop();
	ParseRes->jumpIf(convcond->pos, end);
	then->find();
	ParseRes->jump(start);
	ParseRes->hereStop(end);
}

Type* WhileInstruction::resulttype() {
	return ParseRes->voidType;
}

typedef pair<FunctionDeclaration*,vector<int> > Dist;

void CallInstruction::find() {
	for (int i = 0; i < arguments->size(); i++)
		(*arguments)[i]->find();
	vector<Dist> dists;
	for (map<Funcspec,FunctionDeclaration*>::iterator it = ParseRes->functions.begin(); it != ParseRes->functions.end(); it++) {
		if (it->first.first != *name) // TODO Store functions sorted by name to make this faster
			continue;
		FunctionDeclaration * dec = it->second;
		if (dec->parameters->size() != arguments->size())
			continue;
		dists.push_back(Dist(dec, vector<int>()));
		vector<int> & dv = dists.back().second;
		bool ok = true;
		for (int i = 0; i < arguments->size(); i++) {
			dv.push_back(((*arguments)[i])->resulttype()->distance(dec->parameters->at(i)->type->real()));
			if (dv.back() == INFTY) {
				ok = false;
				break;
			}
		}
		if (!ok)
			dists.pop_back();
	}
	if (dists.empty())
		printerr("Function '%s' (with the specified argument types) does not exist!\n", name->c_str());
	vector<int> smallest(arguments->size(), INFTY);
	for (int i = 0; i < dists.size(); i++) {
		for (int k = 0; k < arguments->size(); k++) {
			smallest[k] = min(smallest[k], dists[i].second[k]);
		}
	}
	dec = 0;
	for (int i = 0; i < dists.size(); i++) {
		bool ok = true;
		for (int k = 0; k < arguments->size(); k++) {
			if (smallest[k] != dists[i].second[k]) {
				ok = false;
				break;
			}
		}
		if (ok)
			dec = dists[i].first;
	}
	if (!dec)
		printerr("Function '%s' cannot be uniquely determined!\n", name->c_str());
	vector<int> args;
	for (int i = 0; i < arguments->size(); i++) {
		Instruction * arg = (*arguments)[i];
		Instruction * convarg = arg->resulttype()->convertTo(arg, (*dec->parameters)[i]->type->real());
		args.push_back(convarg->pos);
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
