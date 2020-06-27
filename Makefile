CFLAGS=-std=c11 -g -static

n9cc: main.c

test: n9cc
	./test.sh

clean:
	rm -f n9cc *.o *~ tmp*

.PHONY: test clean
