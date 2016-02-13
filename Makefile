all: shell

shell: oscode.c
	gcc oscode.c -lm -o shell.o

clean:
	rm -rf *.o shell
