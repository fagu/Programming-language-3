
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "virtualmachine.h"
#include "garbagecollector.h"

//#define PRINTHASH
//#define PRINTSTACK

// FIXME Buffer overflow
char input[10000];
int mainfunc = -1;
vector<int> resultpos;
vector<int> resultsize;
vector<vector<int> > liste;

vector<vector<int> > stops;
vector<vector<int> > argsizes;

#define INSTRUCTION(c,n,code) c,
char opcodes[] = {
#include "vminstructions.cpp"
};
#undef INSTRUCTION
#define INSTRUCTION(c,n,code) n,
int oplength[] = {
#include "vminstructions.cpp"
};
#undef INSTRUCTION

state stat;

int main(int argc, char *argv[]) {
	if (argc >= 2)
		freopen(argv[1], "r", stdin);
	
	int L;
	for (L = 0; true; L++) {
		int c = fgetc(stdin);
		if (c == EOF)
			break;
		if (c == '\n')
			L--;
		else if (c == '|')
			input[L] = '\0';
		else
			input[L] = c;
	}
	input[L] = '\0';
	
	int N = 0;
	char *codepos = input;
	while(*codepos != '\0') {
		char op = *codepos;
		
		if (op == 'M') {
			int stop;
			sscanf(codepos+1, "%d", &stop);
			while(stops.back().size() <= stop)
				stops.back().push_back(0);
			stops.back()[stop] = N;
		} else if (op == 'F' || op == 'f') {
			if (op == 'F')
				mainfunc = liste.size();
			liste.push_back(vector<int>());
			stops.push_back(vector<int>());
			argsizes.push_back(vector<int>());
			N = 0;
			int anzparams;
			int rp, rs;
			sscanf(codepos+1, "%d;%d;%d", &rp, &rs, &anzparams);
			resultpos.push_back(rp);
			resultsize.push_back(rs);
			char *pos;
			for (pos = codepos; *pos != '\0'; pos++)
				if (*pos == ';')
					*pos = '\0';
			char *npos = codepos+strlen(codepos)+1;
			npos = npos+strlen(npos)+1;
			npos = npos+strlen(npos)+1;
			for (int i = 0; i < anzparams; i++) {
				int s;
				sscanf(npos, "%d", &s);
				argsizes.back().push_back(s);
				npos += strlen(npos)+1;
			}
			for (pos--; pos != codepos+1; pos--)
				if (*pos == '\0')
					*pos = ';';
		} else {
			liste.back().push_back(-1);
			for (int i = 0; i < sizeof(opcodes)/sizeof(char); i++)
				if (opcodes[i] == op)
					liste.back()[N] = i;
			if (liste.back()[N] == -1)
				fprintf(stderr, "Unknown command '%c'!\n", op);
			if (op == 'c') {
				int func, resultpos;
				sscanf(codepos+1, "%d;%d", &func, &resultpos);
				liste.back().push_back(func);
				liste.back().push_back(resultpos);
				N += 3;
				char *pos;
				for (pos = codepos; *pos != '\0'; pos++)
					if (*pos == ';')
						*pos = '\0';
				char *npos = codepos+strlen(codepos)+1;
				npos = npos+strlen(npos)+1;
				for (int i = 0; npos != pos+1; i++) {
					int s;
					sscanf(npos, "%d", &s);
					liste.back().push_back(s);
					npos += strlen(npos)+1;
					N++;
				}
				for (pos--; pos != codepos+1; pos--)
					if (*pos == '\0')
						*pos = ';';
			} else if (oplength[liste.back()[N]] == 1) {
				liste.back().push_back(0);
				sscanf(codepos+1, "%d", &liste.back()[N+1]);
				N += 2;
			} else if (oplength[liste.back()[N]] == 2) {
				liste.back().push_back(0); liste.back().push_back(0);
				sscanf(codepos+1, "%d;%d", &liste.back()[N+1], &liste.back()[N+2]);
				N += 3;
			} else if (oplength[liste.back()[N]] == 3) {
				liste.back().push_back(0); liste.back().push_back(0); liste.back().push_back(0);
				sscanf(codepos+1, "%d;%d;%d", &liste.back()[N+1], &liste.back()[N+2], &liste.back()[N+3]);
				N += 4;
			} else if (oplength[liste.back()[N]] == 4) {
				liste.back().push_back(0); liste.back().push_back(0); liste.back().push_back(0); liste.back().push_back(0);
				sscanf(codepos+1, "%d;%d;%d;%d", &liste.back()[N+1], &liste.back()[N+2], &liste.back()[N+3], &liste.back()[N+4]);
				N += 5;
			} else {
				fprintf(stderr, "Unknown command '%c'!\n", op);
			}
		}
		codepos += strlen(codepos)+1;
	}
	
	if (mainfunc == -1)
		fprintf(stderr, "No main function specified!\n");
	
	stat.hash.push_back(0); // Phantom entry to ensure that every pointer is > 0
	stat.hashispointer.push_back(false);
	
	stat.stac.push_back(new stackentry());
	stat.stac.back()->aktpos = 0;
	stat.stac.back()->funcnum = mainfunc;
	stat.stac.back()->copyresultto = -1;
	
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
			switch(op) {
#define INSTRUCTION(c,n,code) case __COUNTER__: code break;
#include "vminstructions.cpp"
#undef INSTRUCTION
				default:
					fprintf(stderr, "Unknown command %d!\n", op);
					return 0;
			}
#ifdef PRINTSTACK
			printf("%d %c (", stat.stac.back()->funcnum, opcodes[op]);
			for (int i = 0; i < oplength[op]; i++) {
				printf("%2d", li[aktpos+i]);
				if (i < oplength[op]-1)
					printf(",");
			}
			printf("):\t ");
			for (int i = 0; i < st.size(); i++) {
				printf("%d ", st[i]);
			}
			printf("\n");
#endif
#ifdef PRINTHASH
			printf("   ");
			for (int i = 0; i < stat.hash.size(); i++) {
				printf("%2d ", stat.hash[i]);
			}
			printf("\n");
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
			}
		}
		stat.stac.pop_back();
stackup:	;
	}
	return 0;
}
