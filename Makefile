all:
	gcc main.c builtins.c executor.c prompt.c aliases.c parser.c command.c -o dsh -lreadline

debug:
	gcc main.c builtins.c executor.c prompt.c aliases.c parser.c command.c -o dsh -lreadline -g -DDEBUG

.PHONY: parser
parser:
	gcc parser.c -DPARSER_ALONE

parserdbg:
	gcc parser.c -DPARSER_ALONE -DDEBUG

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
