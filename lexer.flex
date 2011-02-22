%{
#include <math.h>
#include <string>
#include "bison_parser.h"
%}

DIGIT    [0-9]
ID       [a-zA-Z][a-zA-Z0-9]*

%%

"true" {return TRUE;}
"false" {return FALSE;}
"class" {return CLASS;}

{DIGIT}+ {
	yylval.num = atoi(yytext);
	return NUMBER;
}

{ID} {
	yylval.name = new string(yytext);
	return IDENTIFIER;
}

"#"[^\n]*"\n"     // Einzeilige Kommentare

[ \t\n]+          // Whitespace

. {return yytext[0];}

%%