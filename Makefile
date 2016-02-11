all: felish

felish: oscode.c
	gcc oscode.c -lm -o felish.o

clean:
	rm -rf *.o felish