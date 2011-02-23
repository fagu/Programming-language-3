
#include <stdio.h>
#include <vector>
#include <string.h>
using namespace std;

// TODO Buffer overflow
char input[10000];

vector<char*> stops;

vector<int> stack;
vector<int> hash;

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
		if (*codepos == 'M') {
			int stop;
			sscanf(codepos+1, "%d", &stop);
			while(stops.size() <= stop)
				stops.push_back(0);
			stops[stop] = codepos;
		}
		codepos += strlen(codepos)+1;
	}
	
	codepos = input;
	while(*codepos != '\0') {
		//printf("%s\n", codepos);
		char op = *codepos;
		int len, co, pos, posa, posb, posc;
		char *nextcodepos = codepos + strlen(codepos)+1;
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
			case 'R':
				sscanf(codepos+1, "%d;%d", &len, &pos);
				stack[pos] = hash.size();
				for (int i = 0; i < len; i++)
					hash.push_back(0);
				break;
			case 'S':
				sscanf(codepos+1, "%d;%d;%d", &posa, &posb, &posc);
				hash[stack[posb]+posc] = stack[posa];
				break;
			case 'G':
				sscanf(codepos+1, "%d;%d;%d", &posa, &posb, &posc);
				stack[posc] = hash[stack[posa],posb];
				break;
			case 'J':
				sscanf(codepos+1, "%d;%d", &posa, &posb);
				if (stack[posa] == 0)
					nextcodepos = stops[posb];
				break;
			case 'M':
				break;
			default:
				fprintf(stderr, "Unknown command '%c'!\n", op);
		}
		printf("%s: ", codepos);
		for (int i = 0; i < stack.size(); i++) {
			printf("%d ", stack[i]);
		}
		printf("\n");
		//codepos += strlen(codepos)+1;
		codepos = nextcodepos;
	}
	if (stack.size() != 0)
		fprintf(stderr, "Too much on the stack!\n");
	return 0;
}
