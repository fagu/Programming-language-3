
#include <vector>
#include <stdio.h>
#include "type.h"
#include "build/bison_parser.h"

int yyparse();

int main(int argc, char *argv[]) {
	if (argc >= 2)
		freopen(argv[1], "r", stdin);
	yyparse();
	return 0;
}
