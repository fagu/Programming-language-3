#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include "type.h"
#include "build/bison_parser.h"

int yyparse();

int main(int argc, char *argv[]) {
	system("rm -rf graphs");
	int option;
	while ((option = getopt (argc, argv, "o:g")) != -1) {
		switch (option) {
			case 'o': freopen(optarg, "w", stdout); break;
			case 'g': ParseRes->printgraphs = true; system("mkdir graphs"); break;
		}
	}
	while (optind < argc) {
		freopen(argv[optind], "r", stdin);
		yyparse();
		optind++;
	}
	return ParseRes->output();
}
