#include <algorithm>
#include <queue>
#include <stdio.h>
#include <sstream>
#include "parseresult.h"

ParseResult::ParseResult() {
	haserror = false;
	printgraphs = false;
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
	vector<DeclarationInstruction*> *ve;
	PrimitiveFunction *fu;
	ve = new vector<DeclarationInstruction*>();
	ve->push_back(new DeclarationInstruction(Location(), new TypePointerExplicit(intType), new string("value")));
	fu = new PrimitiveFunction(new string("print_int"), ve, PRINT_INT, new TypePointerExplicit(voidType));
	addPrimitiveFunction(fu);
	ve = new vector<DeclarationInstruction*>();
	ve->push_back(new DeclarationInstruction(Location(), new TypePointerExplicit(charType), new string("value")));
	fu = new PrimitiveFunction(new string("print_char"), ve, PRINT_CHAR, new TypePointerExplicit(voidType));
	addPrimitiveFunction(fu);
	ve = new vector<DeclarationInstruction*>();
	fu = new PrimitiveFunction(new string("dump_stack"), ve, DUMP_STACK, new TypePointerExplicit(voidType));
	addPrimitiveFunction(fu);
	ve = new vector<DeclarationInstruction*>();
	fu = new PrimitiveFunction(new string("dump_heap"), ve, DUMP_HEAP, new TypePointerExplicit(voidType));
	addPrimitiveFunction(fu);
}

void ParseResult::addPrimitiveFunction(Function* func) {
	Funcspec spec;
	spec.first = *func->name;
	for (int i = 0; i < func->parameters->size(); i++) {
		spec.second.push_back((*func->parameters)[i]->type->real());
	}
	functions[spec] = func;
}

void ParseResult::addFunction(FunctionDeclaration* dec) {
	funcdecs.push_back(dec);
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

void ParseResult::binaryoperate(OPCODE o, int a, int b, int c) {
	Node *n = new Node(o, new Arg(GETARG, a, INTSIZE), new Arg(GETARG, b, INTSIZE), new Arg(SETARG, c, INTSIZE));
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

void ParseResult::print(OPCODE o, int from) {
	Node *n = new Node(o, new Arg(GETARG, from, o == PRINT_INT ? INTSIZE : CHARSIZE));
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

void ParseResult::call(Function* func, const std::vector< int >& args, int resultpos) {
	if (func->type() == 'D') {
		Node *n = new Node(CALL, new Arg(INTARG, dynamic_cast<FunctionDeclaration*>(func)->num), new Arg(SETARG, resultpos, func->resulttype->real()->size()));
		for (int i = 0; i < args.size(); i++) {
			n->args.push_back(new Arg(GETARG, args[i], (*func->parameters)[i]->type->real()->size()));
		}
		addnode(n);
	} else {
		Node *n = new Node(dynamic_cast<PrimitiveFunction*>(func)->op);
		if (func->resulttype->real()->size())
			n->args.push_back(new Arg(SETARG, resultpos, func->resulttype->real()->size()));
		for (int i = 0; i < args.size(); i++)
			n->args.push_back(new Arg(GETARG, args[i], (*func->parameters)[i]->type->real()->size()));
		addnode(n);
	}
}

void ParseResult::dump(OPCODE op) {
	Node *n = new Node(op);
	addnode(n);
}

int ParseResult::output() {
	for (vector<ClassType*>::iterator it = classtypes.begin(); it != classtypes.end(); it++)
		(*it)->find();
	int n = 0;
	for (vector<FunctionDeclaration*>::iterator it = funcdecs.begin(); it != funcdecs.end(); it++) {
		FunctionDeclaration *dec = *it;
		Funcspec spec;
		spec.first = *dec->name;
		for (int i = 0; i < dec->parameters->size(); i++) {
			spec.second.push_back((*dec->parameters)[i]->type->real());
		}
		if (functions.count(spec))
			printsyntaxerr(dec->loc, "Multiple definition of function '%s'!\n", dec->name->c_str());
		functions[spec] = dec;
		dec->num = n++;
	}
	for (vector<FunctionDeclaration*>::iterator it = funcdecs.begin(); it != funcdecs.end(); it++) {
		graphs.push_back(new Graph());
		prevnode = graphs.back()->start;
		FunctionDeclaration *dec = *it;
		int retpos = dec->find();
		Node *returnnode;
		if (dec->resulttype->real()->size() > 0)
			returnnode = new Node(RETURN, new Arg(GETARG, retpos));
		else
			returnnode = new Node(RETURN);
		addnode(returnnode);
		graphs.back()->removeOldStops();
		graphs.back()->buildDomTree();
		graphs.back()->convertToSSA();
		graphs.back()->constantPropagation();
		graphs.back()->deadCodeElimination();
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
			system(command);
		}
		
		stringstream str;
		if (*dec->name == "main")
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

