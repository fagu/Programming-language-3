
#include <vector>
#include <stdio.h>
#include "type.h"
#include "build/bison_parser.h"

int yyparse();

int main(int argc, char *argv[]) {
	int option;
	while ((option = getopt (argc, argv, "o:")) != -1) {
		switch (option) {
			case 'o': freopen(optarg, "w", stdout); break;
		}
	}
	while (optind < argc) {
		freopen(argv[optind], "r", stdin);
		yyparse();
		optind++;
	}
	return ParseRes->output();
}
