/**
 * @file		shell_cmd.c
 * @author		Andrew Loebs
 * @brief		Implementation file of the shell command module
 *
 */

#include "shell_cmd.h"

#include <string.h>

#include "shell.h"

// defines
#define MAX_ARGS     8
#define NUM_COMMANDS (sizeof(shell_commands) / sizeof(*shell_commands))

// private types
typedef struct {
    const char *name;
    shell_cmd_handler_t handler;
    const char *help_text;
} shell_command_t;

// private function prototypes
static void command_help(int argc, char *argv[]);

static const shell_command_t *find_command(const char *name);

// constants
static const shell_command_t shell_commands[] = {
    {"help", command_help, "Prints all commands and their associated help text"},
};

// public function definitions
void shell_cmd_process(char *buff, size_t len)
{
    // parse arguments
    char *argv[MAX_ARGS] = {0};
    int argc = 0;
    char *next_arg = NULL;
    for (size_t i = 0; (i < len) && (argc < MAX_ARGS); i++) {
        char *c = &buff[i];
        if ((*c == ' ') || (i == len - 1)) {
            *c = '\0';
            if (next_arg) {
                argv[argc++] = next_arg;
                next_arg = NULL;
            }
        }
        else if (!next_arg) {
            next_arg = c;
        }
    }

    // attempt to find command
    if (argc > 0) {
        const shell_command_t *command = find_command(argv[0]);
        if (command) {
            command->handler(argc, argv);
        }
        else {
            shell_printf_line("Unknown command '%s'.", argv[0]);
            shell_print_line("Type 'help' for a list of commands and descriptions.");
        }
    }
}

// private function definitions
static void command_help(int argc, char *argv[])
{
    for (int i = 0; i < NUM_COMMANDS; i++) {
        shell_printf_line("%s: %s", shell_commands[i].name, shell_commands[i].help_text);
    }
}

static const shell_command_t *find_command(const char *name)
{
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(shell_commands[i].name, name) == 0) return &shell_commands[i];
    }
    // if execution reaches here, we did not find the command
    return NULL;
}