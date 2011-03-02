
#include <stdio.h>
#include <string>
#include <assert.h>
#include "opcodes.h"
#include "virtualmachine.h"
#include "garbagecollector.h"

//#define PRINTHASH
//#define PRINTSTACK

bool showinput = false;
bool run = true;

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

string opname[NUMBEROFOPCODES];

state stat;

int main(int argc, char *argv[]) {
	int option;
	while ((option = getopt (argc, argv, "sn")) != -1) {
		switch (option) {
			case 's': showinput = true; break;
			case 'n': run = false; break;
		}
	}
	if (optind < argc)
		freopen(argv[optind], "r", stdin);
	
	int L;
	for (L = 0; true; L++) {
		int c = fgetc(stdin);
		if (c == EOF)
			break;
		input[L] = c;
	}
	input[L] = '\0';
	
#define INSTRUCTION(c,n,code) opname[c]=string(#c);
#include "vminstructions.cpp"
#undef INSTRUCTION
	for (int i = 0; i < NUMBEROFOPCODES; i++) {
		if (!opname[i].length()) {
			fprintf(stderr, "Opcode %d does not have a name!\n", i);
			return 1;
		}
	}
	
	char *codepos = input;
	while(*codepos != '\0') {
		int op;
		vector<int> args;
		char *pos = codepos;
		while(*pos != ';')
			pos++;
		*pos = '\0';
		sscanf(codepos, "%d", &op);
		pos++;
		while(*pos != '\n') {
			char *scanpos = pos;
			while(*pos != ';')
				pos++;
			*pos = '\0';
			int arg;
			sscanf(scanpos, "%d", &arg);
			args.push_back(arg);
			pos++;
		}
		codepos = pos+1;
		if (showinput) {
			if (op < 0 || op >= NUMBEROFOPCODES) {
				fprintf(stderr, "Operation #%d does not exist!\n", op);
				return 1;
			}
			printf("%25s: ", opname[op].c_str());
			for (int i = 0; i < args.size(); i++)
				printf("%2d ", args[i]);
			printf("\n");
		}
		if (op == FUNC || op == FUNC_MAIN) {
			if (op == FUNC_MAIN)
				mainfunc = liste.size();
			liste.push_back(vector<int>());
			stops.push_back(vector<int>());
			argsizes.push_back(vector<int>());
			resultpos.push_back(args[0]);
			resultsize.push_back(args[1]);
			for (int i = 0; i < args[2]; i++)
				argsizes.back().push_back(args[3+i]);
		} else if (op == HERE_STOP) {
			int stop = args[0];
			while(stops.back().size() <= stop)
				stops.back().push_back(0);
			stops.back()[stop] = liste.back().size();
		} else {
			liste.back().push_back(op);
			for (int i = 0; i < args.size(); i++)
				liste.back().push_back(args[i]);
		}
	}
	
	if (!run)
		return 0;
	
	if (mainfunc == -1) {
		fprintf(stderr, "No main function specified!\n");
		return 1;
	}
	
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
#define INSTRUCTION(c,n,code) case c: code break;
#include "vminstructions.cpp"
#undef INSTRUCTION
				default:
					fprintf(stderr, "Unknown command %d!\n", op);
					return 0;
			}
#ifdef PRINTSTACK
			printf("%2d %s (", stat.stac.back()->funcnum, opname[op].c_str());
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
				nse.ispointer[se.copyresultto+i] = se.ispointer[resultpos[se.funcnum]+i];
			}
		}
		stat.stac.pop_back();
stackup:	;
	}
	return 0;
}
