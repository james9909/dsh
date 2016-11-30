all:
	gcc main.c builtins.c executor.c prompt.c aliases.c parser.c -o dsh -lreadline

debug:
	gcc main.c builtins.c executor.c prompt.c aliases.c parser.c -o dsh -lreadline -g

clean:
	rm -f dsh

run:
	./dsh

.PHONY: test
test:
	for test in tests/*; do \
		clear; \
		./dsh < $$test || exit 1; \
		sleep 2; \
	done
