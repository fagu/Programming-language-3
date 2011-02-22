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
	instr i = {'A', len, lastneeded.size()-len, 0, 0};
	instructions.push_back(i);
	return lastneeded.size()-len;
}

void ParseResult::copy(int from, int len, int to) {
	need(from, len);
	need(to, len);
	instr i = {'C', len, from, to, 0};
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

void ParseResult::output() {
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
		while(lapos < la.size() && la[lapos].first <= i) {
			unocc.push(realpos[la[lapos].second]);
			lapos++;
		}
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
		} else {
			printf("%c%d;%d;%d|\n", in.typ, realpos[in.a], realpos[in.b], realpos[in.c]);
		}
		while(lapos < la.size() && la[lapos].first <= i) {
			unocc.push(realpos[la[lapos].second]);
		}
	}
	printf("A%d|\n", -spacestart);
}

ParseResult *global = new ParseResult();

ParseResult* ParseRes {
	return global;
}

