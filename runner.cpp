#include "runner.h"
#include "opcodes.h"
#include "garbagecollector.h"
#include <stdio.h>

#define BOOLREF(x) (x)
#define CHARREF(x) (x)
#define INTREF(x) (x)

#define dump_stack() do { \
		printf("\033[34mstack: "); \
		for (int i = 0; i < st.size(); i++) { \
			if (ip[i]) \
				printf("p"); \
			printf("%2d ", st[i]); \
		} \
		printf("\033[39m\n"); \
	} while(0)

#define dump_heap() do { \
		printf("\033[32mheap: "); \
		for (int i = 0; i < stat.hash.size(); i++) { \
			if (stat.hashispointer[i]) \
				printf("p"); \
			printf("%2d ", stat.hash[i]); \
		} \
		printf("\033[39m\n"); \
	} while(0)

void run(vector<int> &resultpos, vector<int> &resultsize, vector<vector<int> > &liste, vector<vector<int> > &stops, vector<vector<int> > &argsizes, state &stat) {
	int nextgc = 1;
	
	while(!stat.stac.empty()) {
		stackentry & se = *stat.stac[stat.stac.size()-1];
		vector<int> & st = se.regs;
		vector<bool> & ip = se.ispointer;
		vector<int> & sto = stops[se.funcnum];
		vector<int> & li = liste[se.funcnum];
		int lisize = li.size();
		int aktpos = se.aktpos;
		
		while(aktpos < lisize) {
			char op = li[aktpos];
			int len, co, pos, posa, posb, posc;
			int nextpos;
			aktpos++;
#ifdef PRINTCOMMAND
			printf("\033[31m%2d %s (", stat.stac.back()->funcnum, opname[op].c_str());
			int oplen = oplength[op];
			if (op == CALL)
				oplen = 2+argsizes[li[aktpos]].size();
			for (int i = 0; i < oplen; i++) {
				printf("%2d", li[aktpos+i]);
				if (i < oplen-1)
					printf(",");
			}
			printf("):\033[39m\n");
#endif
			switch(op) {
#define INSTRUCTION(c,n,const,code) case c: code break;
#include "vminstructions.cpp"
#undef INSTRUCTION
				default:
					fprintf(stderr, "Unknown command %d!\n", op);
					return;
			}
#ifdef PRINTSTACK
			dump_stack();
#endif
#ifdef PRINTHEAP
			dump_heap();
#endif
			aktpos = nextpos;
			if (stat.hash.size() >= nextgc) {
				gc(stat);
				nextgc = stat.hash.size()*2;
			}
		}
		if (se.copyresultto != -1) {
			stackentry & nse = *stat.stac[stat.stac.size()-2];
			for (int i = 0; i < resultsize[se.funcnum]; i++) {
				nse.regs[se.copyresultto+i] = se.regs[resultpos[se.funcnum]+i];
				nse.ispointer[se.copyresultto+i] = se.ispointer[resultpos[se.funcnum]+i];
			}
		}
		if (stat.stac.size() == 1)
			return;
		stat.stac.pop_back();
stackup:	;
	}
}
