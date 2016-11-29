all:
	gcc main.c builtins.c executor.c prompt.c aliases.c -o dsh -lreadline

debug:
	gcc main.c builtins.c executor.c prompt.c aliases.c -o dsh -lreadline -g

clean:
	rm -f dsh

run:
	./dsh

test:
	for test in tests/*; do \
		clear; \
		./dsh < $$test; \
		sleep 2; \
	done
