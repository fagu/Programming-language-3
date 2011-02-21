%code requires {
	#include <math.h>
	#include <stdio.h>
	#include <string>
	#include "../parseresult.h"
	#include "../instruction.h"
	using namespace std;
	int yylex (void);
	void yyerror (char const *);
}
%token <name> IDENTIFIER
%token <num> NUMBER
%token TRUE FALSE CLASS
%type <classcontents> classcontents;
%type <classcontent> classcontent;
%type <instruction> exp statement;
%type <statements> statements;
%left '+' '-'
%left '*' '/' '%'
%left '('
%union {
	string *name;
	int num;
	VariableDeclarations *classcontents;
	VariableDeclaration *classcontent;
	Instruction *instruction;
	BlockInstruction *statements;
}

%%

input: outerstatements {
	for (vector<ClassType*>::iterator it = ParseResult::self()->classtypes.begin(); it != ParseResult::self()->classtypes.end(); it++) {
		(*it)->find();
		printf("%d\n", (*it)->size());
	}
}

outerstatements:
	    {
}
	| outerstatements outerstatement {
}

outerstatement:
	  IDENTIFIER IDENTIFIER '(' parameters ')' '{' statements '}' {
	FunctionDeclaration *dec = new FunctionDeclaration($7);
	if (ParseResult::self()->functions.count(*$2))
		fprintf(stderr, "Multiple definition of function '%s'!\n", $2->c_str());
	else
		ParseResult::self()->functions[*$2] = dec;
	$7->find();
	$7->printRPN(stdout);
}
	| CLASS IDENTIFIER '{' classcontents '}' {
	ClassType *type = new ClassType($4);
	ParseResult::self()->classtypes.push_back(type);
	if (ParseResult::self()->types.count(*$2))
		fprintf(stderr, "Multiple definition of type '%s'!\n", $2->c_str());
	else
		ParseResult::self()->types[*$2] = type;
}

statements:
	    {
	$$ = new BlockInstruction();
}
	| statements statement {
	//$2->printRPN(stdout);
	$$->instructions.push_back($2);
}

statement:
	  exp ';' {
	$$ = new PrintInstruction($1);
}
	| IDENTIFIER IDENTIFIER ';' {
	$$ = new DeclarationInstruction(new TypePointer($1), $2);
}
	| IDENTIFIER '=' exp ';' {
	$$ = new SetInstruction($1, $3);
}
	| '{' statements '}' {
	$$ = $2;
}

parameters:
	    {
	//$$ = new vector<string*>();
}
	| neparameters {
	//$$ = $1;
}

neparameters:
	  IDENTIFIER {
	//$$ = new vector<string*>();
	//$$->push_back($1);
}
	| neparameters ',' IDENTIFIER {
	//$1->push_back($3);
	//$$ = $1;
}

classcontents:
	    {
	$$ = new VariableDeclarations();
}
	| classcontents classcontent {
	$1->push_back($2);
	$$ = $1;
}

classcontent:
	  IDENTIFIER IDENTIFIER ';' {
	$$ = new VariableDeclaration($2, new TypePointer($1));
}

exp:
	  '(' exp ')' {
	$$ = $2;
}
	| IDENTIFIER {
	printf("id: %s\n", $1->c_str());
}
	| NUMBER {
	$$ = new IntegerConstantInstruction($1);
	//printf("zahl: %d\n", $1);
}
	| TRUE {
	printf("bool: true\n");
}
	| FALSE {
	printf("bool: false\n");
}
	| exp '+' exp {
	$$ = new BinaryOperatorInstruction('+', $1, $3);
}
	| exp '-' exp {
	$$ = new BinaryOperatorInstruction('-', $1, $3);
}
	| exp '*' exp {
	$$ = new BinaryOperatorInstruction('*', $1, $3);
}
	| exp '/' exp {
	$$ = new BinaryOperatorInstruction('/', $1, $3);
}
	| exp '%' exp {
	$$ = new BinaryOperatorInstruction('%', $1, $3);
}

%%

void yyerror (char const *s) {
	fprintf (stderr, "%s\n", s);
}
