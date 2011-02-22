
#include <stdio.h>
#include <vector>
#include <string.h>
using namespace std;

// TODO Buffer overflow
char input[10000];

vector<int> stack;

int main() {
	int L;
	for (L = 0; true; L++) {
		int c = fgetc(stdin);
		if (c == EOF)
			break;
		if (c == '\n')
			L--;
		else if (c == '|')
			input[L] = '\0';
		else
			input[L] = c;
	}
	input[L] = '\0';
	char *codepos = input;
	while(*codepos != '\0') {
		//printf("%s\n", codepos);
		char op = *codepos;
		int len, co, pos, posa, posb, posc;
		switch(op) {
			case 'A':
				sscanf(codepos+1, "%d", &len);
				if (len > 0) {
					for (int i = 0; i < len; i++)
						stack.push_back(0);
				} else {
					for (int i = 0; i < -len; i++) {
						stack.pop_back();
					}
				}
				break;
			case 'I':
				sscanf(codepos+1, "%d;%d", &co, &pos);
				stack[pos] = co;
				break;
			case '+':
				sscanf(codepos+1, "%d;%d;%d", &posa, &posb, &posc);
				stack[posc] = stack[posa]+stack[posb];
				break;
			case '-':
				sscanf(codepos+1, "%d;%d;%d", &posa, &posb, &posc);
				stack[posc] = stack[posa]-stack[posb];
				break;
			case '*':
				sscanf(codepos+1, "%d;%d;%d", &posa, &posb, &posc);
				stack[posc] = stack[posa]*stack[posb];
				break;
			case '/':
				sscanf(codepos+1, "%d;%d;%d", &posa, &posb, &posc);
				stack[posc] = stack[posa]/stack[posb];
				break;
			case '%':
				sscanf(codepos+1, "%d;%d;%d", &posa, &posb, &posc);
				stack[posc] = stack[posa]%stack[posb];
				break;
			case 'C':
				sscanf(codepos+1, "%d;%d;%d", &posa, &len, &posb);
				for (int i = 0; i < len; i++)
					stack[posb+i] = stack[posa+i];
				break;
			case 'P':
				sscanf(codepos+1, "%d;%d", &pos, &len);
				for (int i = 0; i < len; i++)
					printf("%d ", stack[pos+i]);
				printf("\n");
				break;
			default:
				fprintf(stderr, "Unknown command '%c'!\n", op);
		}
		printf("%s: ", codepos);
		for (int i = 0; i < stack.size(); i++) {
			printf("%d ", stack[i]);
		}
		printf("\n");
		codepos += strlen(codepos)+1;
	}
	if (stack.size() != 0)
		fprintf(stderr, "Too much on the stack!\n");
	return 0;
}
