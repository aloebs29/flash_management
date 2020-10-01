/**
 * @file		shell_cmd.c
 * @author		Andrew Loebs
 * @brief		Implementation file of the shell command module
 *
 */

#include "shell_cmd.h"

#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "spi_nand.h"

// defines
#define MAX_ARGS          8
#define NUM_COMMANDS      (sizeof(shell_commands) / sizeof(*shell_commands))
#define PRINT_BYTES_WIDTH 16

// private types
typedef void (*shell_cmd_handler_t)(int argc, char *argv[]);
typedef struct {
    const char *name;
    shell_cmd_handler_t handler;
    const char *description;
    const char *usage;
} shell_command_t;

// private function prototypes
static void command_help(int argc, char *argv[]);
static void command_read_page(int argc, char *argv[]);

static const shell_command_t *find_command(const char *name);
static void print_bytes(uint8_t *data, size_t len);

// private constants
static const shell_command_t shell_commands[] = {
    {"help", command_help, "Prints all commands and their associated help text.", "help"},
    {"read_page", command_read_page, "Reads a page from the SPI NAND memory unit.",
     "read_page <block> <page> <start byte>"},
};

// private variables
static uint8_t page_buffer[SPI_NAND_PAGE_SIZE];

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
        shell_printf_line("%-12s%s", shell_commands[i].name, shell_commands[i].description);
        shell_printf_line("%12cUsage: %s", ' ', shell_commands[i].usage);
    }
}

static void command_read_page(int argc, char *argv[])
{
    if (argc != 4) {
        shell_printf_line("read_page requires block, page, and start byte arguments. Type \"help\" "
                          "for more info.");
        return;
    }

    // parse arguments
    block_address_t block;
    page_address_t page;
    column_address_t start_byte;
    sscanf(argv[1], "%hu", &block);
    sscanf(argv[2], "%c", &page);
    sscanf(argv[3], "%hu", &start_byte);

    // attempt to read..
    int ret = spi_nand_read_page(block, page, start_byte, page_buffer, sizeof(page_buffer));

    // check for error..
    if (SPI_NAND_RET_OK != ret) {
        shell_printf_line("Error when attempting to read page: %d.", ret);
    }
    else {
        print_bytes(page_buffer, sizeof(page_buffer) - start_byte);
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

static void print_bytes(uint8_t *data, size_t len)
{
    for (int i = 0; i < len; i++) {
        // check if the width has been met
        if ((i % PRINT_BYTES_WIDTH) == (PRINT_BYTES_WIDTH - 1)) {
            shell_printf_line("%02x", data[i]); // print with newline
        }
        else {
            shell_printf("%02x ", data[i]); // print with trailing space
        }
    }
}