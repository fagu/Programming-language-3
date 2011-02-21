/*#include <iostream>

int main(int argc, char **argv) {
    std::cout << "Hello, world!" << std::endl;
    return 0;
}*/
#include <vector>
#include <stdio.h>
#include "type.h"
//#include "CMakeFiles/programmiersprache3.dir/bison_parser.h"
#include "build/bison_parser.h"

/*class Spamklasse {
public:
	int wert;
	Spamklasse() {
		printf("konstruktor\n");
	}
	~Spamklasse() {
		printf("destruktor\n");
	}
	Spamklasse(const Spamklasse &k) {
		printf("kopierer\n");
	}
};

void hallo(Spamklasse k) {
	printf("hallo\n");
}*/

void spam() {
	/*TypePointer p(new string("int"));
	int a = (*p).size();
	int b = p->size();*/
	/*Spamklasse a, b;
	a.wert = 1;
	b.wert = 2;
	//a = Spamklasse(b);
	//b = a;
	//hallo(a);
	printf("%d %d\n", a.wert, b.wert);
	a.wert = 5;
	printf("%d %d\n", a.wert, b.wert);
	b.wert = 6;
	printf("%d %d\n", a.wert, b.wert);*/
}

int yyparse();

int main() {
	//spam();
	yyparse();
	return 0;
}
