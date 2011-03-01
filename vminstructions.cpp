
// Allocate on stack
INSTRUCTION('A',1,
	len = li[aktpos];
	if (len > 0) {
		for (int i = 0; i < len; i++) {
			st.push_back(0);
			ip.push_back(false);
		}
	} else {
		for (int i = 0; i < -len; i++) {
			st.pop_back();
			ip.push_back(false);
		}
	}
	nextpos = aktpos+1;
)

// Integer constant
INSTRUCTION('I',2,
	co = li[aktpos]; pos = li[aktpos+1];
	st[li[aktpos+1]] = li[aktpos];
	ip[li[aktpos+1]] = false;
	nextpos = aktpos+2;
)

// Copy on stack
INSTRUCTION('C',3,
	posa = li[aktpos]; len = li[aktpos+1]; posb = li[aktpos+2];
	for (int i = 0; i < len; i++) {
		st[posb+i] = st[posa+i];
		ip[posb+i] = ip[posa+i];
	}
	nextpos = aktpos+3;
)

// Print
INSTRUCTION('P',2,
	pos = li[aktpos]; len = li[aktpos+1];
	for (int i = 0; i < len; i++)
		printf("%d ", st[pos+i]);
	printf("\n");
	nextpos = aktpos+2;
)

// Allocate on heap and save pointer
INSTRUCTION('R',2,
	pos = li[aktpos]; len = li[aktpos+1];
	st[pos] = alloc(len, stat);
	ip[pos] = true;
	nextpos = aktpos+2;
)

// Set on heap
INSTRUCTION('S',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	stat.hash[st[posb]+posc] = st[posa];
	stat.hashispointer[st[posb]+posc] = ip[posa];
	nextpos = aktpos+3;
)

// Get from heap
INSTRUCTION('G',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = stat.hash[st[posa]+posb];
	ip[posc] = stat.hashispointer[st[posa]+posb];
	nextpos = aktpos+3;
)

// Jump if
INSTRUCTION('J',2,
	posa = li[aktpos]; posb = li[aktpos+1];
	if (st[posa] == 0)
		nextpos = sto[posb];
	else
		nextpos = aktpos+2;
)

// Jump
INSTRUCTION('j',1,
	posa = li[aktpos];
	nextpos = sto[posa];
)

// Call function
INSTRUCTION('c',1,
	co = li[aktpos]; pos = li[aktpos+1];
	stat.stac.push_back(new stackentry());
	stat.stac.back()->aktpos = 0;
	stat.stac.back()->funcnum = co;
	stat.stac.back()->copyresultto = pos;
	se.aktpos = aktpos;
	for (int i = 0; i < argsizes[co].size(); i++) {
		for (int k = 0; k < argsizes[co][i]; k++) {
			stat.stac.back()->regs.push_back(st[li[aktpos+i+2]+k]);
			stat.stac.back()->ispointer.push_back(ip[li[aktpos+i+2]+k]);
		}
		se.aktpos++;
	}
	/*printf("called: (%d): ", stat.stac.back()->regs.size());
	for (int r = 0; r < stat.stac.back()->regs.size(); r++)
		printf("%d ", stat.stac.back()->regs[r]);
	printf("\n");*/
	se.aktpos+=2;
	goto stackup;
)

INSTRUCTION('+',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]+st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('-',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]-st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('*',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]*st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('/',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]/st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('%',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]%st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('=',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] == st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('<',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] < st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('>',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] > st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('(',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] <= st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(')',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] >= st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('~',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] != st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('&',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = (st[posa]!=0 && st[posb]!=0 ? 1 : 0);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION('O',3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = (st[posa]!=0 || st[posb]!=0 ? 1 : 0);
	ip[posc] = false;
	nextpos = aktpos+3;
)
