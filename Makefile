all:
	gcc main.c builtins.c executor.c prompt.c -o dsh -lreadline

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
