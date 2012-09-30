#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <getopt.h>
#include "type.h"
#include "build/bison_parser.h"

int yyparse();

int main(int argc, char *argv[]) {
	init();
	if (system("rm -rf graphs")) {
		fprintf(stderr, "Error removing 'graphs' directory!\n");
		return 1;
	}
	int option;
	while ((option = getopt (argc, argv, "o:g")) != -1) {
		switch (option) {
			case 'o':
				if (!freopen(optarg, "w", stdout)) {
					fprintf(stderr, "Error opening output file '%s'!\n", optarg);
					return 1;
				}
				break;
			case 'g':
				ParseRes->printgraphs = true;
				if (system("mkdir graphs")) {
					fprintf(stderr, "Error creating 'graphs' directory!\n");
					return 1;
				}
				break;
		}
	}
	while (optind < argc) {
		if (!freopen(argv[optind], "r", stdin))
			fprintf(stderr, "Error opening input file '%s'!\n", argv[optind]);
		else
			yyparse();
		optind++;
	}
	return ParseRes->output();
}
