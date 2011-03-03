#include <algorithm>
#include <queue>
#include <stdio.h>
#include <sstream>
#include "parseresult.h"

ParseResult::ParseResult() {
	haserror = false;
	nullType = new NullType(Location());
	voidType = new PrimitiveType(Location(), 0);
	intType = new PrimitiveType(Location(), 1);
	boolType = new PrimitiveType(Location(), 1);
	charType = new PrimitiveType(Location(), 1);
	types["null"] = nullType;
	types["void"] = voidType;
	types["int"] = intType;
	types["bool"] = boolType;
	types["char"] = charType;
}

void ParseResult::need(int id, int len) {
	for (int i = 0; i < len; i++) {
		lastneeded.back()[id+i] = instructions.back().size();
	}
}

int ParseResult::alloc(int len) {
	for (int k = 0; k < len; k++) {
		lastneeded.back().push_back(instructions.back().size());
	}
	instr i = {ALLOC_STACK, len, lastneeded.back().size()-len};
	instructions.back().push_back(i);
	return lastneeded.back().size()-len;
}

void ParseResult::copy(int from, int len, int to) {
	need(from, len);
	need(to, len);
	instr i = {COPY_STACK, len, from, to};
	instructions.back().push_back(i);
}

void ParseResult::binaryoperate(OPCODE o, int a, int b, int c) {
	need(a, intType->size());
	need(b, intType->size());
	need(c, intType->size());
	instr i = {o, 0, a, b, c};
	instructions.back().push_back(i);
}

void ParseResult::intconst(int nr, int to) {
	need(to, intType->size());
	instr i = {INT_CONST, 0, nr, to};
	instructions.back().push_back(i);
}

void ParseResult::charconst(char ch, int to) {
	need(to, charType->size());
	instr i = {CHAR_CONST, 0, ch, to};
	instructions.back().push_back(i);
}

void ParseResult::print(OPCODE o, int from) {
	if (o == PRINT_INT)
		need(from, intType->size());
	else if (o == PRINT_CHAR)
		need(from, charType->size());
	instr i = {o, 0, from};
	instructions.back().push_back(i);
}

void ParseResult::newRef(int len, int to) {
	need(to, intType->size());
	instr i = {ALLOC_HEAP_CONSTAMOUNT, len, to};
	instructions.back().push_back(i);
}

void ParseResult::newArray(int unitsize, int sizepos, int to) {
	need(sizepos, intType->size());
	instr i = {ALLOC_HEAP_VARAMOUNT, unitsize, sizepos, to};
	instructions.back().push_back(i);
}

void ParseResult::getSub(int from, int varid, int to, int len) {
	need(from, len);
	need(to, len);
	instr i = {GET_HEAP, len, from, varid, to};
	instructions.back().push_back(i);
}

void ParseResult::copySub(int from, int to, int varid, int len) {
	need(from, len);
	need(to, len);
	instr i = {SET_HEAP, len, from, to, varid};
	instructions.back().push_back(i);
}

void ParseResult::accessArray(int unitsize, int from, int pos, int to) {
	need(from, intType->size());
	need(pos, intType->size());
	need(to, unitsize);
	instr i = {GET_ARRAY, unitsize, from, pos, to};
	instructions.back().push_back(i);
}

void ParseResult::setArray(int unitsize, int from, int to, int pos) {
	need(from, unitsize);
	need(to, intType->size());
	need(pos, intType->size());
	instr i = {SET_ARRAY, unitsize, from, to, pos};
	instructions.back().push_back(i);
}

int ParseResult::newStop() {
	stoppos.back().push_back(-1);
	return stoppos.back().size()-1;
}

void ParseResult::hereStop(int stop) {
	stoppos.back()[stop] = instructions.back().size()-1;
	instr i = {HERE_STOP, 0, stop};
	instructions.back().push_back(i);
}

void ParseResult::jumpIf(int cond, int stop) {
	need(cond, intType->size());
	instr i = {JUMPIF, 0, cond, stop};
	instructions.back().push_back(i);
}

void ParseResult::jump(int stop) {
	instr i = {JUMP, 0, stop};
	instructions.back().push_back(i);
}

void ParseResult::call(int func, const std::vector< int >& args, int resultpos) {
	instr i = {CALL, 0, func, resultpos};
	i.v = args;
	instructions.back().push_back(i);
}

void ParseResult::dump(OPCODE op) {
	instr i = {op};
	instructions.back().push_back(i);
}

void ParseResult::output() {
	for (vector<ClassType*>::iterator it = classtypes.begin(); it != classtypes.end(); it++)
		(*it)->find();
	
	int n = 0;
	for (map<string,FunctionDeclaration*>::iterator it = functions.begin(); it != functions.end(); it++)
		it->second->num = n++;
	for (map<string,FunctionDeclaration*>::iterator it = functions.begin(); it != functions.end(); it++) {
		lastneeded.push_back(vector<int>());
		instructions.push_back(vector<instr>());
		stoppos.push_back(vector<int>());
		
		FunctionDeclaration *dec = it->second;
		int retpos = dec->find();
		
		stringstream str;
		
		if (it->first == "main")
			str << FUNC_MAIN;
		else
			str << FUNC;
		str << ";" << retpos << ";" << (*dec->resulttype)->size() << ";" << (int)dec->parameters->size() << ";";
		for (int i = 0; i < dec->parameters->size(); i++) {
			str << (*dec->parameters)[i]->type->real()->size() << ";";
		}
		str << "\n";
		funcspecs.push_back(str.str());
	}
	if (haserror)
		return;
	for (int f = 0; f < funcspecs.size(); f++) {
		printf("%s", funcspecs[f].c_str());
		realpos.resize(lastneeded[f].size());
		vector<pair<int,int> > la;
		for (int i = 0; i < lastneeded[f].size(); i++) {
			la.push_back(make_pair(lastneeded[f][i], i));
		}
		sort(la.begin(), la.end());
		int lapos = 0;
		int spacestart = 0;
		//queue<int> unocc;
		for (int i = 0; i < instructions[f].size(); i++) {
			instr in = instructions[f][i];
			if (in.typ == ALLOC_STACK) {
				for (int k = 0; k < in.len; k++) {
					//if (unocc.empty()) {
						realpos[in.a+k] = spacestart;
						spacestart++;
					//} else {
					//	realpos[in.a+k] = unocc.front();
					//	unocc.pop();
					//}
				}
			}
			/*while(lapos < la.size() && la[lapos].first <= i) {
				unocc.push(realpos[la[lapos].second]);
				lapos++;
			}*/
		}
		printf("%d;%d;\n", ALLOC_STACK, spacestart);
		for (int i = 0; i < instructions[f].size(); i++) {
			instr in = instructions[f][i];
			if (in.typ == ALLOC_STACK) {
			} else {
				printf("%d;", in.typ);
				if (in.typ == COPY_STACK) {
					printf("%d;%d;%d;\n", realpos[in.a], in.len, realpos[in.b]);
				} else if (in.typ == PRINT_INT || in.typ == PRINT_CHAR) {
					printf("%d;\n", realpos[in.a]);
				} else if (in.typ == INT_CONST) {
					printf("%d;%d;\n", in.a, realpos[in.b]);
				} else if (in.typ == CHAR_CONST) {
					printf("%d;%d;\n", in.a, realpos[in.b]);
				} else if (in.typ == ALLOC_HEAP_CONSTAMOUNT) {
					printf("%d;%d;\n", realpos[in.a], in.len);
				} else if (in.typ == ALLOC_HEAP_VARAMOUNT) {
					printf("%d;%d;%d;\n", in.len, realpos[in.a], realpos[in.b]);
				} else if (in.typ == GET_HEAP) {
					printf("%d;%d;%d;%d;\n", realpos[in.a], in.b, realpos[in.c], in.len);
				} else if (in.typ == SET_HEAP) {
					printf("%d;%d;%d;%d;\n", realpos[in.a], realpos[in.b], in.c, in.len);
				} else if (in.typ == GET_ARRAY) {
					printf("%d;%d;%d;%d;\n", in.len, realpos[in.a], realpos[in.b], realpos[in.c]);
				} else if (in.typ == SET_ARRAY) {
					printf("%d;%d;%d;%d;\n", in.len, realpos[in.a], realpos[in.b], realpos[in.c]);
				} else if (in.typ == HERE_STOP) {
					printf("%d;\n", in.a);
				} else if (in.typ == JUMPIF) {
					printf("%d;%d;\n", realpos[in.a], in.b);
				} else if (in.typ == JUMP) {
					printf("%d;\n", in.a);
				} else if (in.typ == CALL) {
					printf("%d;%d;", in.a, in.b);
					for (int i = 0; i < in.v.size(); i++)
						printf("%d;", realpos[in.v[i]]);
					printf("\n");
				} else if (in.typ == DUMP_STACK || in.typ == DUMP_HEAP) {
					printf("\n");
				} else {
					printf("%d;%d;%d;\n", realpos[in.a], realpos[in.b], realpos[in.c]);
				}
			}
		}
		//printf("%d;%d;\n", ALLOC_STACK, -spacestart);
		
		realpos.clear();
	}
}

ParseResult *global = new ParseResult();

ParseResult* ParseRes {
	return global;
}

