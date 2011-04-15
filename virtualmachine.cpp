
#include <stdio.h>
#include <string>
#include <assert.h>
#include "opcodes.h"
#include "virtualmachine.h"
#include "runner.h"

#define VIRTUALMACHINE

bool showinput = false;
bool dorun = true;

// FIXME Buffer overflow
char input[1000000];
int mainfunc = -1;
vector<int> resultpos;
vector<int> resultsize;
vector<vector<int> > liste;

vector<vector<int> > stops;
vector<vector<int> > argsizes;

state stat;

int main(int argc, char *argv[]) {
	int option;
	while ((option = getopt (argc, argv, "sno:")) != -1) {
		switch (option) {
			case 's': showinput = true; break;
			case 'n': dorun = false; break;
			case 'o':
				if (!freopen(optarg, "w", stdout)) {
					fprintf(stderr, "Error opening output file '%s'!\n", optarg);
					return 1;
				}
				break;
		}
	}
	if (optind < argc) {
		if (!freopen(argv[optind], "r", stdin)) {
			fprintf(stderr, "Error opening input file '%s'!\n", argv[optind]);
			return 1;
		}
		optind++;
	}
	if (optind < argc) {
		fprintf(stderr, "Too many arguments!\n");
		return 1;
	}
	
	init();
	
	int L;
	for (L = 0; true; L++) {
		int c = fgetc(stdin);
		if (c == EOF)
			break;
		input[L] = c;
	}
	input[L] = '\0';
	
	int staticsize = -1;
	
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
			if (op == FUNC || op == FUNC_MAIN)
				printf("\033[1;31m");
			printf("%25s: ", opname((OPCODE)op).c_str());
			if (op == FUNC || op == FUNC_MAIN)
				printf("\033[22;39m");
			for (int i = 0; i < args.size(); i++)
				printf("%2d ", args[i]);
			printf("\n");
		}
		if (op == ALLOC_STATIC) {
			staticsize = args[0];
		} else if (op == FUNC || op == FUNC_MAIN) {
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
	
	if (!dorun)
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
	
	stat.stati.reserve(staticsize);
	
	run(resultpos, resultsize, liste, stops, argsizes, stat);
	
	return 0;
}
