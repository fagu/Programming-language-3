%code requires {
	#include <math.h>
	#include <stdio.h>
	#include <string>
	#include "../parseresult.h"
	#include "../instruction.h"
	#include "../location.h"
	#include "../function.h"
	#include "../type.h"
	#define YYLTYPE Location
	using namespace std;
	int yylex (void);
	void yyerror (char const *);
}
%token <name> IDENTIFIER
%token <num> NUMBER
%token <character> CHARACTER
%token <name> STRING
%token NUL CLASS NEW IF ELSE WHILE FOR EQ LE GE NE PP MM ARRAY
%type <classtype> classcontents;
%type <vardec> variabledeclaration;
%type <funcdec> functiondeclaration;
%type <instruction> exp narexp statement;
%type <statements> statements;
%type <params> parameters neparameters;
%type <instructions> arguments nearguments;
%type <type> type ttype;
%left LAND LOR
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
	char character;
	ClassType *classtype;
	VariableDeclaration *vardec;
	FunctionDeclaration *funcdec;
	Instruction *instruction;
	BlockInstruction *statements;
	vector<DeclarationInstruction*> * params;
	vector<Instruction*> *instructions;
	TypePointer * type;
}

%expect 3

%%

input: outerstatements {
}

outerstatements:
	    {
}
	| outerstatements outerstatement {
}

outerstatement:
	  functiondeclaration {
	ParseRes->addFunction($1);
}
	| CLASS IDENTIFIER '{' classcontents '}' {
	$4->loc = @$;
	$4->name = $2;
	ParseRes->addClass($4);
}
	| error {
	printsyntaxerr(@$, "Syntax error!\n");
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
	';' {
	$$ = new EmptyInstruction(@$);
}
	| exp ';' {
	$$ = $1;
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
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back(new IntegerConstantInstruction(@$, 1));
	$$ = new SetInstruction(@$, $1, new CallInstruction(@$, new string("operator+"), v));
}
	| exp MM ';' {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back(new IntegerConstantInstruction(@$, 1));
	$$ = new SetInstruction(@$, $1, new CallInstruction(@$, new string("operator-"), v));
}
	| '{' statements '}' {
	$2->loc = @$;
	$$ = $2;
}
	| IF '(' exp ')' statement {
	$$ = new IfInstruction(@$, $3, new BlockInstruction(@5, $5), new BlockInstruction(Location()));
}
	| IF '(' exp ')' statement ELSE statement {
	$$ = new IfInstruction(@$, $3, new BlockInstruction(@5, $5), new BlockInstruction(@7, $7));
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
	| error ';' {
	printsyntaxerr(@$, "syntax error\n");
	$$ = new EmptyInstruction(@$);
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
	$$ = new ClassType();
}
	| classcontents variabledeclaration {
	$1->addVariable($2);
	$$ = $1;
}
	| classcontents functiondeclaration {
	$2->parameters->insert($2->parameters->begin(), new DeclarationInstruction(@2, new TypePointerExplicit($1), new string("this")));
	$1->addFunction($2);
	$2->thisClass = $1;
	$$ = $1;
}

variabledeclaration:
	  type IDENTIFIER ';' {
	$$ = new VariableDeclaration(@$, $2, $1);
}

functiondeclaration:
	  type IDENTIFIER '(' parameters ')' statement {
	$$ = new FunctionDeclaration(@$, $2, $4, new BlockInstruction(@6, $6), $1);
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
	| CHARACTER {
	$$ = new CharacterConstantInstruction(@$, $1);
}
	| STRING {
	$$ = new StringConstantInstruction(@$, $1);
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
	| exp '.' IDENTIFIER '(' arguments ')' {
	$$ = new ClassCallInstruction(@$, $1, $3, $5);
}
	| exp '+' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator+"), v);
}
	| exp '-' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator-"), v);
}
	| exp '*' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator*"), v);
}
	| exp '/' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator/"), v);
}
	| exp '%' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator%"), v);
}
	| exp EQ exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator=="), v);
}
	| exp '<' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator<"), v);
}
	| exp '>' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator>"), v);
}
	| exp LE exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator<="), v);
}
	| exp GE exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator>="), v);
}
	| exp NE exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator!="), v);
}
	| exp LAND exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator&&"), v);
}
	| exp LOR exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back($1); v->push_back($3);
	$$ = new CallInstruction(@$, new string("operator||"), v);
}
	| '+' exp %prec NEG {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back(new IntegerConstantInstruction(@$, 0)); v->push_back($2);
	$$ = new CallInstruction(@$, new string("operator+"), v);
}
	| '-' exp %prec NEG {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back(new IntegerConstantInstruction(@$, 0)); v->push_back($2);
	$$ = new CallInstruction(@$, new string("operator-"), v);
}
	| '!' exp {
	vector<Instruction*> *v = new vector<Instruction*>(); v->push_back(new IntegerConstantInstruction(@$, 0)); v->push_back($2);
	$$ = new CallInstruction(@$, new string("operator!="), v); // TODO faster solution with unary operator
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
	//fprintf (stderr, "%s\n", s);
}
