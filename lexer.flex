%{
#include <math.h>
#include <string>
#include "bison_parser.h"
# define YY_USER_ACTION yylloc.first_line = yylloc.last_line; yylloc.first_column = yylloc.last_column; yylloc.last_column += yyleng;
# define YY_USER_INIT yylloc.first_line = 1; yylloc.first_column = 1; yylloc.last_line = 1; yylloc.last_column = 1;
%}

DIGIT    [0-9]
ID       [a-zA-Z][a-zA-Z0-9]*

%%

"null" {return NUL;}
"class" {return CLASS;}
"new" {return NEW;}
"if" {return IF;}
"while" {return WHILE;}
"for" {return FOR;}
"==" {return EQ;}
"<=" {return LE;}
">=" {return GE;}
"!=" {return NE;}
"&&" {return AND;}
"||" {return OR;}
"++" {return PP;}
"--" {return MM;}
"[]" {return ARRAY;}

{DIGIT}+ {
	yylval.num = atoi(yytext);
	return NUMBER;
}

{ID} {
	yylval.name = new string(yytext);
	return IDENTIFIER;
}

"//"[^\n]*"\n" {
	yylloc.last_line++;
	yylloc.last_column = 1;
}

[ \t]+

[\n]+ {
	yylloc.last_line += yyleng;
	yylloc.last_column = 1;
}

. {return yytext[0];}

%%
