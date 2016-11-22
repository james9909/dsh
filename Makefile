all:
	gcc main.c builtins.c executor.c prompt.c -o dsh

clean:
	rm -f dsh

run:
	./dsh
