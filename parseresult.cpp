#include <algorithm>
#include <queue>
#include <stdio.h>
#include <sstream>
#include "parseresult.h"
#include "function.h"
#include "type.h"
#include "instruction.h"
#include "flowgraph.h"
#include "environment.h"

ParseResult::ParseResult() {
	haserror = false;
	printgraphs = false;
	env = new Environment;
	staticBlock = new BlockInstruction(Location());
	funcnum = 0;
	nullType = new NullType(Location());
	voidType = new PrimitiveType(Location(), 0);
	intType = new PrimitiveType(Location(), INTSIZE);
	boolType = new PrimitiveType(Location(), BOOLSIZE);
	charType = new PrimitiveType(Location(), CHARSIZE);
	types["null"] = nullType;
	types["void"] = voidType;
	types["int"] = intType;
	types["bool"] = boolType;
	types["char"] = charType;
	addPrim("print_int", PRINT_INT, intType, "a", voidType);
	addPrim("print_char", PRINT_CHAR, charType, "a", voidType);
	addPrim("dump_stack", DUMP_STACK, voidType);
	addPrim("dump_heap", DUMP_HEAP, voidType);
	addPrim("operator+", PLUS, intType, "a", intType, "b", intType);
	addPrim("operator-", MINUS, intType, "a", intType, "b", intType);
	addPrim("operator*", TIMES, intType, "a", intType, "b", intType);
	addPrim("operator/", DIV, intType, "a", intType, "b", intType);
	addPrim("operator%", MOD, intType, "a", intType, "b", intType);
	addPrim("operator==", EQUAL, intType, "a", intType, "b", boolType);
	addPrim("operator<", LESS, intType, "a", intType, "b", boolType);
	addPrim("operator>", GREATER, intType, "a", intType, "b", boolType);
	addPrim("operator<=", LESSOREQUAL, intType, "a", intType, "b", boolType);
	addPrim("operator>=", GREATEROREQUAL, intType, "a", intType, "b", boolType);
	addPrim("operator!=", UNEQUAL, intType, "a", intType, "b", boolType);
	addPrim("operator&&", AND, boolType, "a", boolType, "b", boolType);
	addPrim("operator||", OR, boolType, "a", boolType, "b", boolType);
}

void ParseResult::addPrim(string name, OPCODE op, Type* resulttype) {
	vector<Type*> *argTypes = new vector<Type*>;
	FunctionAccessorPrimitive *fa = new FunctionAccessorPrimitive(new string(name), resulttype, op, argTypes);
	env->addFunction(fa);
}

void ParseResult::addPrim(string name, OPCODE op, Type* at, string an, Type* resulttype) {
	vector<Type*> *argTypes = new vector<Type*>;
	argTypes->push_back(at);
	FunctionAccessorPrimitive *fa = new FunctionAccessorPrimitive(new string(name), resulttype, op, argTypes);
	env->addFunction(fa);
}

void ParseResult::addPrim(string name, OPCODE op, Type* at, string an, Type* bt, string bn, Type* resulttype) {
	vector<Type*> *argTypes = new vector<Type*>;
	argTypes->push_back(at);
	argTypes->push_back(bt);
	FunctionAccessorPrimitive *fa = new FunctionAccessorPrimitive(new string(name), resulttype, op, argTypes);
	env->addFunction(fa);
}

void ParseResult::addFunctionDefinition(FunctionDefinition* dec) {
	funcdecs.push_back(dec);
	dec->num = funcnum;
	funcnum++;
}

void ParseResult::addClass(ClassType* cl) {
	classtypes.push_back(cl);
	if (types.count(*cl->name))
		printsyntaxerr(cl->loc, "Multiple definition of type '%s'!\n", cl->name->c_str());
	else
		types[*cl->name] = cl;
	for (vector<FunctionDefinition*>::iterator it = cl->funcs.begin(); it != cl->funcs.end(); it++) {
		funcdecs.push_back(*it);
	}
}

void ParseResult::addVariable(VariableDeclaration* dec) {
	vars.push_back(dec);
}

void ParseResult::addnode(Node* n) {
	if (prevnode)
		prevnode->suc.push_back(n);
	prevnode = n;
}

int ParseResult::alloc(int len) {
	if (len == 0)
		return 0;
	assert(len == 1);
	return graphs.back()->varnum++; // TODO Handle variables with length != 1
}

void ParseResult::copy(int from, int len, int to) {
	Node *n = new Node(COPY_STACK, new Arg(GETARG, from, len), new Arg(INTARG, len), new Arg(SETARG, to, len));
	addnode(n);
}

void ParseResult::intconst(int nr, int to) {
	Node *n = new Node(INT_CONST, new Arg(INTARG, nr), new Arg(SETARG, to, INTSIZE));
	addnode(n);
}

void ParseResult::charconst(char ch, int to) {
	Node *n = new Node(CHAR_CONST, new Arg(INTARG, ch), new Arg(SETARG, to, CHARSIZE));
	addnode(n);
}

void ParseResult::newRef(int len, int to) {
	Node *n = new Node(ALLOC_HEAP_CONSTAMOUNT, new Arg(SETARG, to, POINTERSIZE), new Arg(INTARG, len));
	addnode(n);
}

void ParseResult::newArray(int unitsize, int sizepos, int to) {
	Node *n = new Node(ALLOC_HEAP_VARAMOUNT, new Arg(INTARG, unitsize), new Arg(GETARG, sizepos, INTSIZE), new Arg(SETARG, to, POINTERSIZE));
	addnode(n);
}

void ParseResult::getSub(int from, int varid, int to, int len) {
	Node *n = new Node(GET_HEAP, new Arg(GETARG, from, POINTERSIZE), new Arg(INTARG, varid), new Arg(SETARG, to, len), new Arg(INTARG, len));
	addnode(n);
}

void ParseResult::copySub(int from, int to, int varid, int len) {
	Node *n = new Node(SET_HEAP, new Arg(GETARG, from, len), new Arg(GETARG, to, POINTERSIZE), new Arg(INTARG, varid), new Arg(INTARG, len));
	addnode(n);
}

void ParseResult::accessArray(int unitsize, int from, int pos, int to) {
	Node *n = new Node(GET_ARRAY, new Arg(INTARG, unitsize), new Arg(GETARG, from, POINTERSIZE), new Arg(GETARG, pos, INTSIZE), new Arg(SETARG, to, unitsize));
	addnode(n);
}

void ParseResult::setArray(int unitsize, int from, int to, int pos) {
	Node *n = new Node(SET_ARRAY, new Arg(INTARG, unitsize), new Arg(GETARG, from, unitsize), new Arg(GETARG, to, POINTERSIZE), new Arg(GETARG, pos, INTSIZE));
	addnode(n);
}

int ParseResult::newStop() {
	Node *n = new Node(HERE_STOP);
	stops.push_back(n);
	return stops.size()-1;
}

void ParseResult::hereStop(int stop) {
	addnode(stops[stop]);
}

void ParseResult::jumpIf(int cond, int stop) {
	Node *n = new Node(JUMPIF, new Arg(GETARG, cond, BOOLSIZE));
	n->suc.push_back(stops[stop]);
	addnode(n);
}

void ParseResult::jump(int stop) {
	prevnode->suc.push_back(stops[stop]);
	prevnode = 0;
}

void ParseResult::call(int funcnum, const vector<int>& args, const vector<int> &argsizes, int resultpos, int resultsize) {
	Node *n = new Node(CALL, new Arg(INTARG, funcnum), new Arg(SETARG, resultpos, resultsize));
	for (int i = 0; i < args.size(); i++) {
		n->args.push_back(new Arg(GETARG, args[i], argsizes[i]));
	}
	addnode(n);
}

void ParseResult::call(OPCODE op, const vector<int>& args, const vector<int>& argsizes, int resultpos, int resultsize) {
	Node *n = new Node(op);
	for (int i = 0; i < args.size(); i++)
		n->args.push_back(new Arg(GETARG, args[i], argsizes[i]));
	if (resultsize)
		n->args.push_back(new Arg(SETARG, resultpos, resultsize));
	addnode(n);
}

void ParseResult::callPointer(int fppos, vector<int> args, vector<int> argsizes, int resultpos, int resultsize) {
	Node *n = new Node(CALL_POINTER, new Arg(INTARG, fppos), new Arg(SETARG, resultpos, resultsize));
	for (int i = 0; i < args.size(); i++) {
		n->args.push_back(new Arg(GETARG, args[i], argsizes[i]));
	}
	addnode(n);
}

void ParseResult::getStatic(int from, int len, int to) {
	Node *n = new Node(GET_STATIC, new Arg(INTARG, from), new Arg(INTARG, len), new Arg(SETARG, to, len));
	addnode(n);
}

void ParseResult::setStatic(int from, int len, int to) {
	Node *n = new Node(SET_STATIC, new Arg(GETARG, from, len), new Arg(INTARG, len), new Arg(INTARG, to));
	addnode(n);
}

int ParseResult::output() {
	FunctionDefinition *staticFunc = new FunctionDefinition(Location(), new vector<DeclarationInstruction*>, staticBlock, new TypePointerExplicit(voidType));
	addFunctionDefinition(staticFunc);
	for (vector<ClassType*>::iterator it = classtypes.begin(); it != classtypes.end(); it++)
		(*it)->find();
	int globalsize = 0;
	for (vector<VariableDeclaration*>::iterator it = vars.begin(); it != vars.end(); it++) {
		(*it)->pos = globalsize;
		globalsize += (*it)->type->real()->size();
		env->addVariable(new VariableAccessorStatic((*it)->name, *it/*(*it)->pos)*/));
	}
	int n = 1;
	for (vector<FunctionDefinition*>::iterator it = funcdecs.begin(); it != funcdecs.end(); it++) {
		FunctionDefinition *dec = *it;
		dec->num = n++;
	}
	/*for (vector<FunctionDefinition*>::iterator it = funcglob.begin(); it != funcglob.end(); it++) {
		FunctionDefinition *dec = *it;
		vector<Type*> *argTypes = new vector<Type*>;
		for (int i = 0; i < dec->parameters->size(); i++)
			argTypes->push_back((*dec->parameters)[i]->type->real());
		// FunctionAccessorNormal *fa = new FunctionAccessorNormal(dec->name, dec->resulttype->real(), dec->num, argTypes, 0);
		FunctionType *fpt = new FunctionType(Location(), dec->resulttype->real(), argTypes);
		VariableAccessorStatic *fpa = new VariableAccessorStatic(dec->name, fpt, globalsize);
		globalsize += fpt->size();
		FunctionAccessorPointer *fa = new FunctionAccessorPointer(dec->name, dec->resulttype->real(), fpa, argTypes, 0);
		env->addFunction(fa);
	}*/
	for (vector<ClassType*>::iterator it = classtypes.begin(); it != classtypes.end(); it++)
		(*it)->findFuncs();
	vector<string> funcspecs;
	//for (vector<FunctionDefinition*>::iterator it = funcdecs.begin(); it != funcdecs.end(); it++) {
	for (int i = 0; i < funcdecs.size(); i++) {
		graphs.push_back(new Graph());
		prevnode = graphs.back()->start;
		//FunctionDefinition *dec = *it;
		FunctionDefinition *dec = funcdecs[i];
		int retpos = dec->find(env);
		Node *returnnode;
		if (dec->resulttype->real()->size() > 0)
			returnnode = new Node(RETURN, new Arg(GETARG, retpos));
		else
			returnnode = new Node(RETURN);
		addnode(returnnode);
		graphs.back()->removeOldStops();
		graphs.back()->buildDomTree();
		//graphs.back()->convertToSSA();
		//graphs.back()->constantPropagation();
		//graphs.back()->deadCodeElimination();
		//graphs.back()->livenessAnalysis();
		graphs.back()->addNewStops();
		
		if (printgraphs) {
			char filename[100];
			sprintf(filename, "graphs/%s-%d.dot", dec->name->c_str(), dec->num);
			FILE *fi = fopen(filename, "w");
			graphs.back()->printGraph(fi);
			fclose(fi);
			char command[1000];
			sprintf(command, "dot graphs/%s-%d.dot -Tpng -o graphs/%s-%d.png", dec->name->c_str(), dec->num, dec->name->c_str(), dec->num);
			if (system(command))
				fprintf(stderr, "Error creating the flowgraph (is 'dot' installed?)\n");
		}
		
		stringstream str;
		if (*dec->name == "main" || dec == staticFunc)
			str << FUNC_MAIN;
		else
			str << FUNC;
		str << ";" << retpos << ";" << (*dec->resulttype)->size() << ";" << (int)dec->parameters->size() << ";";
		for (int i = 0; i < dec->parameters->size(); i++) {
			str << (*dec->parameters)[i]->type->real()->size() << ";";
		}
		str << "\n";
		str << ALLOC_STACK << ";" << graphs.back()->varnum << ";\n";
		funcspecs.push_back(str.str());
		stops.clear();
	}
	if (haserror)
		return 1;
	printf("%d;%d;\n", ALLOC_STATIC, globalsize);
	for (int f = 0; f < funcspecs.size(); f++) {
		printf("%s", funcspecs[f].c_str());
		graphs[f]->start->print();
	}
	return 0;
}

ParseResult *global = new ParseResult();

ParseResult* ParseRes {
	return global;
}

