LIBS = -lreadline

FILES= \
	main.c \
	builtins.c \
	executor.c \
	prompt.c \
	aliases.c \
	parser.c \
	command.c

all:
	gcc $(FILES) $(LIBS) -o dsh

debug:
	gcc $(FILES) $(LIBS) -g -DDEBUG -o dsh

.PHONY: parser
parser:
	gcc parser.c -DPARSER_ALONE

parserdbg:
	gcc parser.c -DPARSER_ALONE -DDEBUG

clean:
	rm -f dsh a.out

run:
	./dsh

.PHONY: test
test:
	for test in tests/*; do \
		clear; \
		./dsh < $$test || exit 1; \
		sleep 2; \
	done
