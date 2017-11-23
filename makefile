
default: lrt

lrt.o: lrt.c
	gcc -c lrt.c -o lrt.o

lrt: lrt.o
	gcc lrt.o -o lrt

clean:
	-rm lrt.o
	-rm lrt
