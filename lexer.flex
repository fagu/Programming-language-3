%{
#include <stdio.h>
#include <math.h>
#include <string>
#include "bison_parser.h"
# define YY_USER_ACTION yylloc.first_line = yylloc.last_line; yylloc.first_column = yylloc.last_column; yylloc.last_column += yyleng;
# define YY_USER_INIT yylloc.first_line = 1; yylloc.first_column = 1; yylloc.last_line = 1; yylloc.last_column = 1;
%}

DIGIT    [0-9]
ID       [a-zA-Z_][a-zA-Z0-9_]*

%x ST_COMMENT
%x ST_STRING

%%

	int commentdepth;
	string *str;
	int fl, fc;

"null" {return NUL;}
"class" {return CLASS;}
"new" {return NEW;}
"if" {return IF;}
"else" {return ELSE;}
"while" {return WHILE;}
"for" {return FOR;}
"==" {return EQ;}
"<=" {return LE;}
">=" {return GE;}
"!=" {return NE;}
"&&" {return LAND;}
"||" {return LOR;}
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

"//".*"\n" {
	yylloc.last_line++;
	yylloc.last_column = 1;
}

"/*" {
	commentdepth = 1;
	BEGIN(ST_COMMENT);
}

<ST_COMMENT>{
"/*" {
	commentdepth++;
}
"*/" {
	commentdepth--;
	if (commentdepth == 0)
		BEGIN(0);
}
.
\n+ {
	yylloc.last_line += yyleng;
	yylloc.last_column = 1;
}
}

\'[^\\\'\n]\'	yylval.character = yytext[1]; return CHARACTER;
\'\\'\'	yylval.character = '\''; return CHARACTER;
\'\\n\'	yylval.character = '\n'; return CHARACTER;
\'\\t\'	yylval.character = '\t'; return CHARACTER;
\'\\\\\'	yylval.character = '\\'; return CHARACTER;

\" {
	BEGIN(ST_STRING);
	str = new string;
	fl = yylloc.first_line;
	fc = yylloc.first_column;
}

<ST_STRING>{
\" {
	BEGIN(0);
	yylval.name = str;
	yylloc.first_line = fl;
	yylloc.first_column = fc;
	return STRING;
}
\\\"	str->append("\"");
\\n	str->append("\n");
\\t	str->append("\t");
\\\\	str->append("\\");
\\	str->append("\\");
\n {
	fprintf(stderr, "%d: String not terminated in this line!\n", yylloc.first_line);
	ParseRes->haserror = true;
	BEGIN(0);
	yylloc.last_line++;
	yylloc.last_column = 1;
	yylloc.first_line = fl;
	yylloc.first_column = fc;
	yylval.name = str;
	return STRING;
}
[^\\\"\n]	str->append(yytext);
}

[ \t]+

[\n]+ {
	yylloc.last_line += yyleng;
	yylloc.last_column = 1;
}

. {return yytext[0];}

%%
