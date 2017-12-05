all: loadgen cfs

loadgen: loadgen.c
	gcc -g -Wall -o loadgen loadgen.c -std=c99 -lm 

cfs: cfs.c 
	gcc -g -Wall -o cfs  cfs.c -std=c99

clean:
	/bin/rm -fr *~ *.o cfs loadgen
