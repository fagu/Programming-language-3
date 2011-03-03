#!/bin/bash
cd ..
timelimit -T 1 -t 2 build/compiler tests/$1 -o tests/$1.code;
if (($?)); then
	echo "Could not compile!";
	exit 1;
fi;
timelimit -T 1 -t 5 build/vm tests/$1.code -o tests/$1.out
if (($?)); then
	echo "Could not run!";
	exit 1;
fi;
cmp tests/$1.ok tests/$1.out
if (($?)); then
	echo "Wrong output!";
	exit 1;
fi;
exit 0;