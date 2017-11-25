
default: lrt

lrt: lrt.c
	gcc -g -O0 lrt.c -o lrt

clean:
	-rm lrt
