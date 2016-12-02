LIBS = -lreadline

FILES= \
	builtins.c \
	executor.c \
	prompt.c \
	aliases.c \
	parser.c \
	command.c

all:
	gcc main.c $(FILES) $(LIBS) -o dsh

debug:
	gcc main.c $(FILES) $(LIBS) -g -o dsh

.PHONY: parser
parser:
	gcc $(FILES) $(LIBS) -DPARSER_ALONE

parserdbg:
	gcc $(FILES) $(LIBS) -DPARSER_ALONE -DDEBUG

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
