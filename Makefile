all: felish

felish: oscode.c
	gcc oscode.c -lm -o felish

clean:
	rm -rf *.o felish