## dsh

The dead-inside shell - Nobel Gautam and James Wang

Features:
* Parsing of multiple commands in one line
* Interactive input using GNU Readline
* Support for quoted arguments
* Support for redirection
* Support for piping together 2 or more commands
* Customizeable prompt
* Support for globs
* Support for aliases
* Support for setting environment variables
* Configuration file for aliases and prompt
* Support for running commands in the background with '&'
* Recursive descent parser

TODO:

Attempted:

Bugs:

Files & Function Headers:

`main.c`

Contains the loop that prints the prompt, reads, and executes user input.
```c
/* Handles signals. Will send the signal to the currently running child process, if it exists. */
static void handler(int signo);

/* Executes all commands found in the configuration file */
void load_config();

/* Retrieves user input and executes it */
int main();
```

`aliases.c`

Contains functions related to aliases
```c

/*
 * Check if an alias exists
 *
 * Returns the index of the alias if it exists, and -1 otherwise
 */
int find_alias(char *name);

/*
 * Adds an alias to the table. Will overwrite any existing aliases with the same name
 */
void add_alias(char *alias, char *replacement);

/*
 * Replaces the first argument of a command with its alias, if it exists.
 */
void handle_aliases(Command *c);
```

`builtins.c`

Contains shell builtins
```c

/*
 * Expands the tilde in a path.
 * path must be freed manually.
 */
void expand_path(char **path);

/*
 * Checks if a command is a builtin.
 *
 * Returns 1 if c is a builtin, 0 otherwise.
 */
int is_builtin(Command *c);

/*
 * Handles all shell built-in commands that are not meant to be
 * executed with exec.
 */
void handle_builtins(Command *C);
```

`prompt.c`

Handles the rendering of the prompt.
```c

/* Sets the global variable 'exit_code' to n */
void set_exit_code(int n);

/* Returns the exit code of the last command ran */
int get_exit_code();

/*
 * Loads the custom prompt and checks its validity.
 * If the given prompt is invalid, then a default one is used instead.
 */
void load_prompt();

/*
 * Gets information about the current git repository.
 * Must be manually freed.
 *
 * Returns a string in the format (<branch> <status>)
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

/* Executes a single command */
void exec(Command *c);

/* Goes through c, executing all commands */
void run(Command *c);

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
