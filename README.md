## dsh

The dead-inside shell - Nobel Gautam and James Wang

Features:
* Parsing of multiple commands in one line
* Bash-like keyboard shortcuts
* Tab completion
* Quoted args
* Support for redirection
* Support for piping together 2 or more commands
* Customizeable prompt
* Support for globs
* Support for aliases (limitation: no semicolons in definition)
* Add config file for aliases and prompt
* Recursive descent parser

TODO:
* Conditionals/loops

Attempted:

Bugs:

Files & Function Headers:
`main.c`

Contains the loop that prints the prompt, reads, and executes user input.

`builtins.c`

Contains the builtin commands `cd` and `exit`.
```c

/* Expands all strings in an array.
 *
 * Returns a dynamically allocated string array.
 */
char **expand(char **path);

/*
 * Handles all shell built-in commands that are not meant to be
 * executed with exec.
 *
 * Returns 1 if the built-in exists, 0 otherwise.
 */
int handle_builtins(char *argl[]);
```

`prompt.c`

Handles the rendering of the prompt.
```c

/* Sets the global variable 'exit_code' to the given parameter  */
void set_exit_code(int n);

/*
 * Loads the custom prompt and checks its validity.
 * If the given prompt is invalid, then a default one is used instead.
 */
void load_prompt();

/*
 * Gets information for the current git repository.
 *
 * Returns a dynamically allocated string in the format (<branch> <status>)
 */
char *git_info();

/*
 * Returns a dynamically allocated string containing the rendered prompt, with all
 * variables substituted.
 */
char *get_prompt();

/*
 * Returns a dynamically allocated string containing the value associated with a specific variable
 */
char *get_variable();
```

`executor.c`

Handles the execution of commands

```c

/*
 * Trims all single-space strings from a string array
 * Other functions set arguments to single space as a means of removal
 * This function allows that to work.
 */
void remove_spaces(char *argv[512]);

/*
 * Looks for args with quotes in them
 * and combines them into a single arg.
 * Also copies everything over to heap
 * (must be manually freed).
 */
void combine_quoted(char *argv[512]);

/*
 * Check if any redirection needs to be done,
 * and perform them accordingly
 */
void handle_redirect(char *argv[512]);

/*
 * Runs commands that contain pipes.
 * Mirrors the normal program flow but
 * pipes stdin/stdout properly
 */
void handle_pipes(char *cmd, int num_pipes);

/* Runs a command */
void run(char *input);

/* Sends a signal to the currently running process, if it exists */
void signal_process(int signo);
```

`parser.c`

Handles parsing. Uses input to create a tree of commands and their relations.

```c

/*
 * If the given character is the next element
 * in the input, discard it and return true.
 * Otherwise return false.
 */
int accept(char c);

/*
 * accept(char) but for strings
 * Used for "&&" and "||"
 */
int accepts(char *c);

/* Discard until first non-whitespace character */
void ignore_whitespace();

/*
 * If the given character is the next element
 * in the input, discard it and return true.
 * Otherwise, abort.
 * Currently unused.
 */
void expect(char c);

/*
 * Read a word up till certain special characters
 * or until quote ends. Put on heap and make *dst point to it.
 */
int parse_word(char **dst);

/* Parse all forms of redirects. */
int parse_redirect(Command *a);

/* Attempt to parse redirects and commands */
int parse_expr(Command *a);

/* Call parse_expr until nothing left to parse */
int parse_exprs(Command *a);

/* Currently useless. Adds another layer */
int parse_cmd(Command *a);

/* Parse pipes. Update current command to the piped one. */
int parse_pipe(Command **a);

/* Parse && and ||. Update current command to the new one. */
int parse_connector(Command *a);

/* Parse ;. Update current command to the new one. */
int parse_cmdlist(Command *a);

/* Free all Commands linked forwardly */
void free_cmds(Command *c);

/* DEBUG. Print all fields in a Command */
void print_cmd(Command *c);

/* DEBUG. Print all fields of all linked Commands */
void print_cmds(Command *c);

/* Parse input */
void parse(chat *input);
