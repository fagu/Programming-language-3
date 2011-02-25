#include <algorithm>
#include <queue>
#include <stdio.h>
#include "parseresult.h"

ParseResult::ParseResult() {
	voidType = new PrimitiveType(0);
	intType = new PrimitiveType(1);
	boolType = new PrimitiveType(1);
	types["void"] = voidType;
	types["int"] = intType;
	types["bool"] = boolType;
}

void ParseResult::need(int id, int len) {
	for (int i = 0; i < len; i++) {
		lastneeded[id+i] = instructions.size();
	}
}

int ParseResult::alloc(int len) {
	for (int k = 0; k < len; k++) {
		lastneeded.push_back(instructions.size());
	}
	instr i = {'A', len, lastneeded.size()-len};
	instructions.push_back(i);
	return lastneeded.size()-len;
}

void ParseResult::copy(int from, int len, int to) {
	need(from, len);
	need(to, len);
	instr i = {'C', len, from, to};
	instructions.push_back(i);
}

void ParseResult::binaryoperate(char o, int a, int b, int c) {
	need(a, 1);
	need(b, 1);
	need(c, 1);
	instr i = {o, 0, a, b, c};
	instructions.push_back(i);
}

void ParseResult::intconst(int nr, int to) {
	need(to, 1);
	instr i = {'I', 0, nr, to};
	instructions.push_back(i);
}

void ParseResult::print(int from, int len) {
	need(from, len);
	instr i = {'P', len, from};
	instructions.push_back(i);
}

void ParseResult::newRef(int len, int to) {
	need(to, 1);
	instr i = {'R', len, to};
	instructions.push_back(i);
}

void ParseResult::getSub(int from, int varid, int to) {
	need(from, 1);
	need(to, 1);
	instr i = {'G', 0, from, varid, to};
	instructions.push_back(i);
}

void ParseResult::copySub(int from, int to, int varid) {
	need(from, 1);
	need(to, 1);
	instr i = {'S', 0, from, to, varid};
	instructions.push_back(i);
}

int ParseResult::newStop() {
	stoppos.push_back(-1);
	return stoppos.size()-1;
}

void ParseResult::hereStop(int stop) {
	stoppos[stop] = instructions.size()-1;
	instr i = {'M', 0, stop};
	instructions.push_back(i);
}

void ParseResult::jumpIf(int cond, int stop) {
	need(cond, 1);
	instr i = {'J', 0, cond, stop};
	instructions.push_back(i);
}

void ParseResult::jump(int stop) {
	instr i = {'j', 0, stop};
	instructions.push_back(i);
}

void ParseResult::call(int func, const std::vector< int >& args) {
	instr i = {'c', 0, func};
	i.v = args;
	instructions.push_back(i);
}

void ParseResult::output() {
	int n = 0;
	for (map<string,FunctionDeclaration*>::iterator it = functions.begin(); it != functions.end(); it++)
		it->second->num = n++;
	for (map<string,FunctionDeclaration*>::iterator it = functions.begin(); it != functions.end(); it++) {
		FunctionDeclaration *dec = it->second;
		dec->find();
		
		if (it->first == "main")
			printf("F");
		else
			printf("f");
		printf("%d", (int)dec->parameters->size());
		for (int i = 0; i < dec->parameters->size(); i++) {
			printf(";%d", (*dec->parameters)[i]->type->real()->size());
		}
		printf("|\n");
		
		realpos.resize(lastneeded.size());
		vector<pair<int,int> > la;
		for (int i = 0; i < lastneeded.size(); i++) {
			la.push_back(make_pair(lastneeded[i], i));
		}
		sort(la.begin(), la.end());
		int lapos = 0;
		int spacestart = 0;
		queue<int> unocc;
		for (int i = 0; i < instructions.size(); i++) {
			instr in = instructions[i];
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
		for (int i = 0; i < instructions.size(); i++) {
			instr in = instructions[i];
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
			} else if (in.typ == 'G') {
				printf("G%d;%d;%d|\n", realpos[in.a], in.b, realpos[in.c]);
			} else if (in.typ == 'S') {
				printf("S%d;%d;%d|\n", realpos[in.a], realpos[in.b], in.c);
			} else if (in.typ == 'M') {
				printf("M%d|\n", in.a);
			} else if (in.typ == 'J') {
				printf("J%d;%d|\n", realpos[in.a], in.b);
			} else if (in.typ == 'j') {
				printf("j%d|\n", in.a);
			} else if (in.typ == 'c') {
				printf("c%d", in.a);
				for (int i = 0; i < in.v.size(); i++)
					printf(";%d", realpos[in.v[i]]);
				printf("|\n");
			} else {
				printf("%c%d;%d;%d|\n", in.typ, realpos[in.a], realpos[in.b], realpos[in.c]);
			}
		}
		printf("A%d|\n", -spacestart);
		
		lastneeded.clear();
		realpos.clear();
		instructions.clear();
		stoppos.clear();
	}
}

ParseResult *global = new ParseResult();

ParseResult* ParseRes {
	return global;
}

