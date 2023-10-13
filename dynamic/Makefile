all: libdynamic.so

dynamic.o: dynamic.c
	gcc -c -fPIC dynamic.c -o dynamic.o

libdynamic.so: dynamic.o
	gcc -shared dynamic.o -o libdynamic.so

clean:
	rm -f dynamic.o libdynamic.so