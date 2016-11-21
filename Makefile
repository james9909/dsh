all:
	gcc main.c builtins.c -o dsh

clean:
	rm dsh
