
#include <stdio.h>
#include "instruction.h"
#include "parseresult.h"
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
	if (t->style() != Type::STYLE_CLASS)
		printerr("Type '%s' is not a class!\n", name->c_str());
	type = (ClassType*)t;
	pos = ParseRes->alloc(ParseRes->intType->size());
	ParseRes->newRef(type->hashsize(), pos);
}

Type* NewInstruction::resulttype() {
	return type;
}

void DeclarationInstruction::find(Environment* e) {
	varpos = ParseRes->alloc((*type)->size());
	e->addVariable(new VariableAccessorStack(name, type->real(), varpos));
}

Type* DeclarationInstruction::resulttype() {
	return ParseRes->voidType;
}

int DeclarationInstruction::getPos(Environment* e, Instruction* par) {
	return pos;
}

void SetInstruction::find(Environment* e) {
	b->find(e);
	a->findSet(e, b);
}

Type* SetInstruction::resulttype() {
	return ParseRes->voidType;
}

void VariableInstruction::find(Environment* e) {
	Environment::MESSAGE msg;
	acc = e->findVariable(*name, msg);
	if (msg == Environment::NONEFOUND)
		printerr("Variable '%s' does not exist!\n", name->c_str());
	else if (msg == Environment::MULTIPLEFOUND)
		printerr("Variable '%s' exists multiple times!\n", name->c_str());
	acc->find(e, 0);
	pos = acc->pos;
}

Type* VariableInstruction::resulttype() {
	return acc->resulttype();
}

void VariableInstruction::findSet(Environment* e, Instruction* b) {
	Environment::MESSAGE msg;
	acc = e->findVariable(*name, msg);
	if (msg == Environment::NONEFOUND)
		printerr("Variable '%s' does not exist!\n", name->c_str());
	else if (msg == Environment::MULTIPLEFOUND)
		printerr("Variable '%s' exists multiple times!\n", name->c_str());
	acc->findSet(e, 0, b);
}

void ExplicitVariableInstruction::find(Environment* e) {
	acc->find(e, 0);
	pos = acc->pos;
}

Type* ExplicitVariableInstruction::resulttype() {
	return acc->resulttype();
}

void ExplicitVariableInstruction::findSet(Environment* e, Instruction* b) {
	acc->findSet(e, 0, b);
}

void AccessInstruction::find(Environment* e) {
	a->find(e);
	if (a->resulttype()->style() != Type::STYLE_CLASS)
		printerr("Expression is not a class!\n");
	Environment::MESSAGE msg;
	acc = ((ClassType*)a->resulttype())->env->findVariable(*name, msg);
	if (msg == Environment::NONEFOUND)
		printerr("Variable '%s' does not exist!\n", name->c_str());
	else if (msg == Environment::MULTIPLEFOUND)
		printerr("Variable '%s' exists multiple times!\n", name->c_str());
	acc->find(e, a);
	pos = acc->pos;
}

Type* AccessInstruction::resulttype() {
	return acc->resulttype();
}

void AccessInstruction::findSet(Environment* e, Instruction* b) {
	a->find(e);
	if (a->resulttype()->style() != Type::STYLE_CLASS)
		printerr("Expression is not a class!\n");
	Environment::MESSAGE msg;
	acc = ((ClassType*)a->resulttype())->env->findVariable(*name, msg);
	if (msg == Environment::NONEFOUND)
		printerr("Variable '%s' does not exist!\n", name->c_str());
	else if (msg == Environment::MULTIPLEFOUND)
		printerr("Variable '%s' exists multiple times!\n", name->c_str());
	acc->findSet(e, a, b);
}

void AccessArrayInstruction::find(Environment* e) {
	a->find(e);
	b->find(e);
	if (a->resulttype()->style() != Type::STYLE_ARRAY)
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
	if (a->resulttype()->style() != Type::STYLE_ARRAY)
		printerr("Expression is not an array!\n");
	int unitsize = ((ArrayType*)a->resulttype())->contenttype->size();
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
	f->find(e);
	if (f->resulttype()->style() != Type::STYLE_FUNCTION)
		printerr("This is not a function!\n");
	FunctionType *ft = dynamic_cast<FunctionType*>(f->resulttype());
	for (int i = 0; i < arguments->size(); i++) {
		(*arguments)[i]->find(e);
		if ((*arguments)[i]->resulttype()->distance((*ft->argTypes)[i]) == INFTY)
			printerr("Wrong argument type!\n");
	}
	vector<int> argposes;
	vector<int> argsizes;
	/*if (parType) {
		if (!par) {
			Environment::MESSAGE msg;
			VariableAccessor *th = e->findVariable("this", msg);
			th->find(e, 0);
			par = th;
		}
		Instruction *convPar = par->resulttype()->convertTo(par, parType);
		convPar->find(e);
		argposes.push_back(convPar->pos);
		argsizes.push_back(parType->size());
		delete convPar;
	}*/
	for (int i = 0; i < arguments->size(); i++) {
		Expression * arg = (*arguments)[i];
		Instruction * convarg = arg->resulttype()->convertTo(arg, (*ft->argTypes)[i]);
		convarg->find(e);
		argposes.push_back(convarg->pos);
		argsizes.push_back((*ft->argTypes)[i]->size());
		delete convarg;
	}
	pos = ParseRes->alloc(ft->returnType->size());
	ParseRes->callPointer(f->pos, argposes, argsizes, pos, ft->returnType->size());
}

Type* CallInstruction::resulttype() {
	return dynamic_cast<FunctionType*>(f->resulttype())->returnType;
}

/*void CallInstruction::find(Environment* e) {
	vector<Type*> argtypes;
	vector<Expression*> args;
	for (int i = 0; i < arguments->size(); i++) {
		Instruction * arg = (*arguments)[i];
		arg->find(e);
		argtypes.push_back(arg->resulttype());
		args.push_back(arg);
	}
	Environment::MESSAGE message;
	acc = e->findFunction(name, argtypes, message);
	if (message == Environment::NONEFOUND)
		printerr("Function '%s' (with the specified argument types) does not exist!\n", name->c_str());
	else if (message == Environment::MULTIPLEFOUND)
		printerr("Function '%s' (with the specified argument types) cannot be uniquely determined!\n", name->c_str());
	acc->find(e, 0, args);
	pos = acc->pos;
}

Type* CallInstruction::resulttype() {
	return acc->resulttype();
}*/

/*void ClassCallInstruction::find(Environment* e) {
	a->find(e);
	if (a->resulttype()->style() != Type::STYLE_CLASS)
		printerr("Expression is not a class!\n");
	vector<Type*> argtypes;
	vector<Expression*> args;
	for (int i = 0; i < arguments->size(); i++) {
		Instruction * arg = (*arguments)[i];
		arg->find(e);
		argtypes.push_back(arg->resulttype());
		args.push_back(arg);
	}
	Environment::MESSAGE message;
	acc = ((ClassType*)a->resulttype())->env->findFunction(name, argtypes, message);
	if (message == Environment::NONEFOUND)
		printerr("Member function '%s' (with the specified argument types) does not exist!\n", name->c_str());
	else if (message == Environment::MULTIPLEFOUND)
		printerr("Member function '%s' (with the specified argument types) cannot be uniquely determined!\n", name->c_str());
	acc->find(e, a, args);
	pos = acc->pos;
}

Type* ClassCallInstruction::resulttype() {
	return acc->resulttype();
}*/

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

void FunctionDefinitionInstruction::find(Environment* e) {
	ParseRes->addFunctionDefinition(func);
	pos = ParseRes->alloc(INTSIZE);
	ParseRes->intconst(func->num, pos);
}

Type* FunctionDefinitionInstruction::resulttype() {
	return func->funcType()->real();
}



void VariableAccessorStack::findSet(Environment* e, Expression* par, Expression *s) {
	if (s->resulttype()->distance(type) == INFTY)
		printheisenerr("Types do not match!\n");
	Instruction *sn = s->resulttype()->convertTo(s, type);
	sn->find(e);
	ParseRes->copy(sn->pos, type->size(), pos);
	delete sn;
}

void VariableAccessorHeap::find(Environment* e, Expression* par) {
	if (!par) {
		Environment::MESSAGE msg;
		VariableAccessor *th = e->findVariable("this", msg);
		th->find(e, 0);
		par = th;
	}
	pos = ParseRes->alloc(type->size());
	ParseRes->getSub(par->pos, posin, pos, type->size());
}

void VariableAccessorHeap::findSet(Environment* e, Expression* par, Expression* s) {
	if (!par) {
		Environment::MESSAGE msg;
		VariableAccessor *th = e->findVariable("this", msg);
		th->find(e, 0);
		par = th;
	}
	if (s->resulttype()->distance(type) == INFTY)
		printheisenerr("Types do not match!\n");
	Instruction *sn = s->resulttype()->convertTo(s, type);
	sn->find(e);
	ParseRes->copySub(sn->pos, par->pos, posin, type->size());
	delete sn;
}

void VariableAccessorStatic::find(Environment* e, Expression* par) {
	pos = ParseRes->alloc(dec->type->real()->size());
	ParseRes->getStatic(dec->pos/*globpos*/, dec->type->real()->size(), pos);
}

void VariableAccessorStatic::findSet(Environment* e, Expression* par, Expression* s) {
	if (s->resulttype()->distance(dec->type->real()) == INFTY)
		printheisenerr("Types do not match!\n");
	Instruction *sn = s->resulttype()->convertTo(s, dec->type->real());
	sn->find(e);
	ParseRes->setStatic(sn->pos, dec->type->real()->size(), dec->pos/*globpos*/);
	delete sn;
}

Type* VariableAccessorStatic::resulttype() {
	return dec->type->real();
}




void FunctionAccessorNormal::find(Environment* e, Expression* par, const std::vector< Expression* >& args) {
	pos = ParseRes->alloc(type->size());
	vector<int> argposes;
	vector<int> argsizes;
	if (parType) {
		if (!par) {
			Environment::MESSAGE msg;
			VariableAccessor *th = e->findVariable("this", msg);
			th->find(e, 0);
			par = th;
		}
		Instruction *convPar = par->resulttype()->convertTo(par, parType);
		convPar->find(e);
		argposes.push_back(convPar->pos);
		argsizes.push_back(parType->size());
		delete convPar;
	}
	for (int i = 0; i < args.size(); i++) {
		Expression * arg = args[i];
		Instruction * convarg = arg->resulttype()->convertTo(arg, (*argtypes)[i]);
		convarg->find(e);
		argposes.push_back(convarg->pos);
		argsizes.push_back((*argtypes)[i]->size());
		delete convarg;
	}
	pos = ParseRes->alloc(type->size());
	ParseRes->call(funcnum, argposes, argsizes, pos, type->size());
}

void FunctionAccessorPrimitive::find(Environment* e, Expression* par, const std::vector< Expression* >& args) {
	pos = ParseRes->alloc(type->size());
	vector<int> argposes;
	vector<int> argsizes;
	for (int i = 0; i < args.size(); i++) {
		Expression * arg = args[i];
		Instruction * convarg = arg->resulttype()->convertTo(arg, (*argtypes)[i]);
		convarg->find(e);
		argposes.push_back(convarg->pos);
		argsizes.push_back((*argtypes)[i]->size());
		delete convarg;
	}
	pos = ParseRes->alloc(type->size());
	ParseRes->call(op, argposes, argsizes, pos, type->size());
}

void FunctionAccessorPointer::find(Environment* e, Expression* par, const std::vector< Expression* >& args) {
	vector<int> argposes;
	vector<int> argsizes;
	if (parType) {
		if (!par) {
			Environment::MESSAGE msg;
			VariableAccessor *th = e->findVariable("this", msg);
			th->find(e, 0);
			par = th;
		}
		Instruction *convPar = par->resulttype()->convertTo(par, parType);
		convPar->find(e);
		argposes.push_back(convPar->pos);
		argsizes.push_back(parType->size());
		delete convPar;
	}
	fp->find(e, par);
	for (int i = 0; i < args.size(); i++) {
		Expression * arg = args[i];
		Instruction * convarg = arg->resulttype()->convertTo(arg, (*argtypes)[i]);
		convarg->find(e);
		argposes.push_back(convarg->pos);
		argsizes.push_back((*argtypes)[i]->size());
		delete convarg;
	}
	pos = ParseRes->alloc(type->size());
	ParseRes->callPointer(fp->pos, argposes, argsizes, pos, type->size());
}
