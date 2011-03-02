%code requires {
	#include <math.h>
	#include <stdio.h>
	#include <string>
	#include "../parseresult.h"
	#include "../instruction.h"
	#include "../location.h"
	#define YYLTYPE Location
	using namespace std;
	int yylex (void);
	void yyerror (char const *);
}
%token <name> IDENTIFIER
%token <num> NUMBER
%token NUL CLASS NEW IF WHILE FOR EQ LE GE NE PP MM ARRAY
%type <classcontents> classcontents;
%type <classcontent> classcontent;
%type <instruction> exp narexp statement;
%type <statements> statements;
%type <params> parameters neparameters;
%type <instructions> arguments nearguments;
%type <type> type ttype;
%left AND OR
%left '!'
%left EQ LE GE '<' '>' NE
%left '+' '-'
%left '*' '/' '%'
%left NEG
%left '.'
%left '(' '[' '{'
%union {
	string *name;
	int num;
	VariableDeclarations *classcontents;
	VariableDeclaration *classcontent;
	Instruction *instruction;
	BlockInstruction *statements;
	vector<DeclarationInstruction*> * params;
	vector<Instruction*> *instructions;
	TypePointer * type;
}

%expect 2

%%

input: outerstatements {
	ParseRes->output();
}

outerstatements:
	    {
}
	| outerstatements outerstatement {
}

outerstatement:
	  type IDENTIFIER '(' parameters ')' statement {
	FunctionDeclaration *dec = new FunctionDeclaration($4, new BlockInstruction(@6, $6), $1);
	if (ParseResult::self()->functions.count(*$2))
		fprintf(stderr, "Multiple definition of function '%s'!\n", $2->c_str());
	else
		ParseResult::self()->functions[*$2] = dec;
}
	| CLASS IDENTIFIER '{' classcontents '}' {
	ClassType *type = new ClassType(@$, $2, $4);
	ParseResult::self()->classtypes.push_back(type);
	if (ParseResult::self()->types.count(*$2))
		fprintf(stderr, "Multiple definition of type '%s'!\n", $2->c_str());
	else
		ParseResult::self()->types[*$2] = type;
}

statements:
	    {
	$$ = new BlockInstruction(@$);
}
	| statements statement {
	$$->instructions.push_back($2);
	$$->loc = @$;
}

statement:
	  exp ';' {
	$$ = new PrintInstruction(@$, $1);
}
	| type IDENTIFIER ';' {
	$$ = new DeclarationInstruction(@$, $1, $2);
}
	| type IDENTIFIER '=' exp ';' {
	vector<Instruction*> *i = new vector<Instruction*>();
	i->push_back(new DeclarationInstruction(@1+@2, $1, $2));
	i->push_back(new SetInstruction(@2+@3+@4+@5, new VariableInstruction(@2, $2), $4));
	$$ = new CompoundInstruction(@$, i);
}
	| exp '=' exp ';' {
	$$ = new SetInstruction(@$, $1, $3);
}
	| exp PP ';' {
	$$ = new SetInstruction(@$, $1, new BinaryOperatorInstruction(@$, '+', $1, new IntegerConstantInstruction(@$, 1)));
}
	| exp MM ';' {
	$$ = new SetInstruction(@$, $1, new BinaryOperatorInstruction(@$, '-', $1, new IntegerConstantInstruction(@$, 1)));
}
	| '{' statements '}' {
	$2->loc = @$;
	$$ = $2;
}
	| IF '(' exp ')' statement {
	$$ = new IfInstruction(@$, $3, new BlockInstruction(@5, $5));
}
	| WHILE '(' exp ')' statement {
	$$ = new WhileInstruction(@$, $3, new BlockInstruction(@5, $5));
}
	| FOR '(' statement exp ';' statement ')' statement {
	BlockInstruction *a = new BlockInstruction(@$, $3);
	BlockInstruction *b = new BlockInstruction(@$, $8);
	b->instructions.push_back($6);
	a->instructions.push_back(new WhileInstruction(@$, $4, b));
	$$ = a;
}

parameters:
	    {
	$$ = new vector<DeclarationInstruction*>();
}
	| neparameters {
	$$ = $1;
}

neparameters:
	  type IDENTIFIER {
	$$ = new vector<DeclarationInstruction*>();
	$$->push_back(new DeclarationInstruction(@$, $1, $2));
}
	| neparameters ',' type IDENTIFIER {
	$1->push_back(new DeclarationInstruction(@$, $3, $4));
	$$ = $1;
}

arguments:
	    {
	$$ = new vector<Instruction*>();
}
	| nearguments {
	$$ = $1;
}

nearguments:
	  exp {
	$$ = new vector<Instruction*>();
	$$->push_back($1);
}
	| nearguments ',' exp {
	$1->push_back($3);
	$$ = $1;
}

classcontents:
	    {
	$$ = new VariableDeclarations();
}
	| classcontents classcontent {
	if ($1->count(*$2->name))
		fprintf(stderr, "Multiple declaration of '%s'!\n", $2->name->c_str());
	(*$1)[*$2->name] = $2;
	$$ = $1;
}

classcontent:
	  type IDENTIFIER ';' {
	$$ = new VariableDeclaration(@$, $2, $1);
}

exp:      narexp {
	$$ = $1;
}
	| NEW ttype '[' exp ']' {
	$$ = new CreateArrayInstruction(@$, $2, $4);
}
	| NEW IDENTIFIER '[' exp ']' {
	$$ = new CreateArrayInstruction(@$, new TypePointerId(@2, $2), $4);
}
	| NEW IDENTIFIER {
	$$ = new NewInstruction(@$, $2);
}

narexp:
	  '(' exp ')' {
	$$ = $2;
}
	| IDENTIFIER {
	$$ = new VariableInstruction(@$, $1);
}
	| NUMBER {
	$$ = new IntegerConstantInstruction(@$, $1);
}
	| NUL {
	$$ = new NullInstruction(@$);
}
	| narexp '[' exp ']' {
	$$ = new AccessArrayInstruction(@$, $1, $3);
}
	| exp '.' IDENTIFIER {
	$$ = new AccessInstruction(@$, $1, $3);
}
	| IDENTIFIER '(' arguments ')' {
	$$ = new CallInstruction(@$, $1, $3);
}
	| exp '+' exp {
	$$ = new BinaryOperatorInstruction(@$, '+', $1, $3);
}
	| exp '-' exp {
	$$ = new BinaryOperatorInstruction(@$, '-', $1, $3);
}
	| exp '*' exp {
	$$ = new BinaryOperatorInstruction(@$, '*', $1, $3);
}
	| exp '/' exp {
	$$ = new BinaryOperatorInstruction(@$, '/', $1, $3);
}
	| exp '%' exp {
	$$ = new BinaryOperatorInstruction(@$, '%', $1, $3);
}
	| exp EQ exp {
	$$ = new BinaryOperatorInstruction(@$, '=', $1, $3);
}
	| exp '<' exp {
	$$ = new BinaryOperatorInstruction(@$, '<', $1, $3);
}
	| exp '>' exp {
	$$ = new BinaryOperatorInstruction(@$, '>', $1, $3);
}
	| exp LE exp {
	$$ = new BinaryOperatorInstruction(@$, '(', $1, $3);
}
	| exp GE exp {
	$$ = new BinaryOperatorInstruction(@$, ')', $1, $3);
}
	| exp NE exp {
	$$ = new BinaryOperatorInstruction(@$, '~', $1, $3);
}
	| exp AND exp {
	$$ = new BinaryOperatorInstruction(@$, '&', $1, $3);
}
	| exp OR exp {
	$$ = new BinaryOperatorInstruction(@$, 'O', $1, $3);
}
	| '+' exp %prec NEG {
	$$ = new BinaryOperatorInstruction(@$, '+', new IntegerConstantInstruction(@$, 0), $2);
}
	| '-' exp %prec NEG {
	$$ = new BinaryOperatorInstruction(@$, '-', new IntegerConstantInstruction(@$, 0), $2);
}
	| '!' exp {
	$$ = new BinaryOperatorInstruction(@$, '~', $2, new IntegerConstantInstruction(@$, 0)); // TODO faster solution with unary operator
}

type:
	  IDENTIFIER {
	$$ = new TypePointerId(@$, $1);
}
	| ttype {
	$$ = $1;
}

ttype:
	  type ARRAY {
	$$ = new TypePointerArray(@$, $1);
}

%%

void yyerror (char const *s) {
	fprintf (stderr, "%s\n", s);
}
