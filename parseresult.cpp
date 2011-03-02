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
	types["null"] = nullType;
	types["void"] = voidType;
	types["int"] = intType;
	types["bool"] = boolType;
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
	instr i = {'A', len, lastneeded.back().size()-len};
	instructions.back().push_back(i);
	return lastneeded.back().size()-len;
}

void ParseResult::copy(int from, int len, int to) {
	need(from, len);
	need(to, len);
	instr i = {'C', len, from, to};
	instructions.back().push_back(i);
}

void ParseResult::binaryoperate(char o, int a, int b, int c) {
	need(a, 1);
	need(b, 1);
	need(c, 1);
	instr i = {o, 0, a, b, c};
	instructions.back().push_back(i);
}

void ParseResult::intconst(int nr, int to) {
	need(to, 1);
	instr i = {'I', 0, nr, to};
	instructions.back().push_back(i);
}

void ParseResult::print(int from, int len) {
	need(from, len);
	instr i = {'P', len, from};
	instructions.back().push_back(i);
}

void ParseResult::newRef(int len, int to) {
	need(to, 1);
	instr i = {'R', len, to};
	instructions.back().push_back(i);
}

void ParseResult::newArray(int unitsize, int sizepos, int to) {
	need(sizepos, 1);
	instr i = {'r', unitsize, sizepos, to};
	instructions.back().push_back(i);
}

void ParseResult::getSub(int from, int varid, int to) {
	need(from, 1);
	need(to, 1);
	instr i = {'G', 0, from, varid, to};
	instructions.back().push_back(i);
}

void ParseResult::copySub(int from, int to, int varid) {
	need(from, 1);
	need(to, 1);
	instr i = {'S', 0, from, to, varid};
	instructions.back().push_back(i);
}

void ParseResult::accessArray(int unitsize, int from, int pos, int to) {
	need(from, 1);
	need(pos, 1);
	need(to, unitsize);
	instr i = {'x', unitsize, from, pos, to};
	instructions.back().push_back(i);
}

void ParseResult::setArray(int unitsize, int from, int to, int pos) {
	need(from, unitsize);
	need(to, 1);
	need(pos, 1);
	instr i = {'X', unitsize, from, to, pos};
	instructions.back().push_back(i);
}

int ParseResult::newStop() {
	stoppos.back().push_back(-1);
	return stoppos.back().size()-1;
}

void ParseResult::hereStop(int stop) {
	stoppos.back()[stop] = instructions.back().size()-1;
	instr i = {'M', 0, stop};
	instructions.back().push_back(i);
}

void ParseResult::jumpIf(int cond, int stop) {
	need(cond, 1);
	instr i = {'J', 0, cond, stop};
	instructions.back().push_back(i);
}

void ParseResult::jump(int stop) {
	instr i = {'j', 0, stop};
	instructions.back().push_back(i);
}

void ParseResult::call(int func, const std::vector< int >& args, int resultpos) {
	instr i = {'c', 0, func, resultpos};
	i.v = args;
	instructions.back().push_back(i);
}

void ParseResult::dump(char op) {
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
			str << "F"; //printf("F");
		else
			str << "f"; //printf("f");
		str << retpos << ";" << (*dec->resulttype)->size() << ";" << (int)dec->parameters->size();
		//printf("%d;%d;%d", retpos, (*dec->resulttype)->size(), (int)dec->parameters->size());
		for (int i = 0; i < dec->parameters->size(); i++) {
			str << ";" << (*dec->parameters)[i]->type->real()->size();
		//	printf(";%d", (*dec->parameters)[i]->type->real()->size());
		}
		str << "|\n";
		//printf("|\n");
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
		queue<int> unocc;
		for (int i = 0; i < instructions[f].size(); i++) {
			instr in = instructions[f][i];
			if (in.typ == 'A') {
				for (int k = 0; k < in.len; k++) {
					if (unocc.empty()) {
						realpos[in.a+k] = spacestart;
						spacestart++;
					} else {
						realpos[in.a+k] = unocc.front();
						unocc.pop();
					}
				}
			}
			/*while(lapos < la.size() && la[lapos].first <= i) {
				unocc.push(realpos[la[lapos].second]);
				lapos++;
			}*/
		}
		printf("A%d|\n", spacestart);
		for (int i = 0; i < instructions[f].size(); i++) {
			instr in = instructions[f][i];
			if (in.typ == 'A') {
			} else if (in.typ == 'C') {
				printf("C%d;%d;%d|\n", realpos[in.a], in.len, realpos[in.b]);
			} else if (in.typ == 'P') {
				for (int k = 0; k < in.len; k++) {
					printf("P%d;1|\n", realpos[in.a+k]);
				}
			} else if (in.typ == 'I') {
				printf("I%d;%d|\n", in.a, realpos[in.b]);
			} else if (in.typ == 'R') {
				printf("R%d;%d|\n", realpos[in.a], in.len);
			} else if (in.typ == 'r') {
				printf("r%d;%d;%d|\n", in.len, realpos[in.a], realpos[in.b]);
			} else if (in.typ == 'G') {
				printf("G%d;%d;%d|\n", realpos[in.a], in.b, realpos[in.c]);
			} else if (in.typ == 'S') {
				printf("S%d;%d;%d|\n", realpos[in.a], realpos[in.b], in.c);
			} else if (in.typ == 'x') {
				printf("x%d;%d;%d;%d|\n", in.len, realpos[in.a], realpos[in.b], realpos[in.c]);
			} else if (in.typ == 'X') {
				printf("X%d;%d;%d;%d|\n", in.len, realpos[in.a], realpos[in.b], realpos[in.c]);
			} else if (in.typ == 'M') {
				printf("M%d|\n", in.a);
			} else if (in.typ == 'J') {
				printf("J%d;%d|\n", realpos[in.a], in.b);
			} else if (in.typ == 'j') {
				printf("j%d|\n", in.a);
			} else if (in.typ == 'c') {
				printf("c%d;%d", in.a, in.b);
				for (int i = 0; i < in.v.size(); i++)
					printf(";%d", realpos[in.v[i]]);
				printf("|\n");
			} else if (in.typ == 'D' || in.typ == 'd') {
				printf("%c|\n", in.typ);
			} else {
				printf("%c%d;%d;%d|\n", in.typ, realpos[in.a], realpos[in.b], realpos[in.c]);
			}
		}
		printf("A%d|\n", -spacestart);
		
		realpos.clear();
	}
}

ParseResult *global = new ParseResult();

ParseResult* ParseRes {
	return global;
}

