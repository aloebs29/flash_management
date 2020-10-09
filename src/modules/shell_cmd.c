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
static void command_write_page(int argc, char *argv[]);
static void command_erase_block(int argc, char *argv[]);
static void command_get_bad_block_table(int argc, char *argv[]);
static void command_mark_bad_block(int argc, char *argv[]);
static void command_page_is_free(int argc, char *argv[]);

static const shell_command_t *find_command(const char *name);
static void print_bytes(uint8_t *data, size_t len);

// private constants
static const shell_command_t shell_commands[] = {
    {"help", command_help, "Prints all commands and their associated help text.", "help"},
    {"read_page", command_read_page, "Reads a page from the SPI NAND memory unit.",
     "read_page <block> <page> <column>"},
    {"write_page", command_write_page,
     "Writes a value (repeated) to a page of the SPI NAND memory unit.",
     "write_page <block> <page> <column> <value>"},
    {"erase_block", command_erase_block, "Erases a block of the SPI NAND memory unit.",
     "erase_block <block>"},
    {"get_bbt", command_get_bad_block_table,
     "Prints a 0 (good) or 1 (bad) representing each block's status.", "get_bbt"},
    {"mark_bb", command_mark_bad_block, "Marks a block of the SPI NAND memory unit as bad.",
     "mark_bb <block>"},
    {"page_is_free", command_page_is_free, "Determines whether a page is free.",
     "page_is_free <block> <page>"},
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
        shell_printf_line("%-14s%s", shell_commands[i].name, shell_commands[i].description);
        shell_printf_line("%14cUsage: %s", ' ', shell_commands[i].usage);
    }
}

static void command_read_page(int argc, char *argv[])
{
    if (argc != 4) {
        shell_printf_line("read_page requires block, page, and column arguments. Type \"help\" "
                          "for more info.");
        return;
    }

    // parse arguments
    uint16_t block, page, column;
    sscanf(argv[1], "%hu", &block);
    sscanf(argv[2], "%hu", &page);
    sscanf(argv[3], "%hu", &column);

    // attempt to read..
    row_address_t row = {.block = block, .page = page};
    int ret = spi_nand_page_read(row, column, page_buffer, sizeof(page_buffer));

    // check for error..
    if (SPI_NAND_RET_OK != ret) {
        shell_printf_line("Error when attempting to read page: %d.", ret);
    }
    else {
        print_bytes(page_buffer, sizeof(page_buffer) - column);
    }
}

static void command_write_page(int argc, char *argv[])
{
    if (argc != 5) {
        shell_printf_line(
            "write_page requires block, page, column, and value arguments. Type \"help\" "
            "for more info.");
        return;
    }

    // parse arguments
    uint16_t block, page, column, value;
    sscanf(argv[1], "%hu", &block);
    sscanf(argv[2], "%hu", &page);
    sscanf(argv[3], "%hu", &column);
    sscanf(argv[4], "%hu", &value);

    // create write data
    size_t write_len = sizeof(page_buffer) - column;
    memset(page_buffer, value, write_len);
    // attempt to write..
    row_address_t row = {.block = block, .page = page};
    int ret = spi_nand_page_program(row, column, page_buffer, write_len);

    // check for error..
    if (SPI_NAND_RET_OK != ret) {
        shell_printf_line("Error when attempting to write page: %d.", ret);
    }
    else {
        shell_printf_line("Write page successful.");
    }
}

static void command_erase_block(int argc, char *argv[])
{
    if (argc != 2) {
        shell_printf_line("erase_block requires a block number as an argument. Type \"help\" "
                          "for more info.");
        return;
    }

    // parse arguments
    uint16_t block;
    sscanf(argv[1], "%hu", &block);

    // attempt to erase..
    row_address_t row = {.block = block, .page = 0};
    int ret = spi_nand_block_erase(row);

    // check for error..
    if (SPI_NAND_RET_OK != ret) {
        shell_printf_line("Error when attempting to erase block: %d.", ret);
    }
    else {
        shell_printf_line("Block erase successful.");
    }
}

static void command_get_bad_block_table(int argc, char *argv[])
{
    // read block status into page buffer
    for (int i = 0; i < SPI_NAND_BLOCKS_PER_LUN; i++) {
        bool is_bad;
        row_address_t row = {.block = i, .page = 0};
        int ret = spi_nand_block_is_bad(row, &is_bad);
        if (SPI_NAND_RET_OK != ret) {
            // error occured
            shell_printf_line("Error when checking block %d status: %d.", i, ret);
            return; // exit
        }
        else {
            page_buffer[i] = is_bad;
        }
    }

    // print bad block table
    print_bytes(page_buffer, SPI_NAND_BLOCKS_PER_LUN);
}

static void command_mark_bad_block(int argc, char *argv[])
{
    if (argc != 2) {
        shell_printf_line("mark_bb requires a block number as an argument. Type \"help\" "
                          "for more info.");
        return;
    }

    // parse arguments
    uint16_t block;
    sscanf(argv[1], "%hu", &block);

    // attempt to mark bad block..
    row_address_t row = {.block = block, .page = 0};
    int ret = spi_nand_block_mark_bad(row);

    // check for error..
    if (SPI_NAND_RET_OK != ret) {
        shell_printf_line("Error when attempting to mark bad block: %d.", ret);
    }
    else {
        shell_printf_line("Mark bad block successful.");
    }
}

static void command_page_is_free(int argc, char *argv[])
{
    if (argc != 3) {
        shell_printf_line("page_is_free requires block and page arguments. Type \"help\" "
                          "for more info.");
        return;
    }

    // parse arguments
    uint16_t block, page;
    sscanf(argv[1], "%hu", &block);
    sscanf(argv[2], "%hu", &page);

    // attempt to get is free status..
    bool is_free;
    row_address_t row = {.block = block, .page = page};
    int ret = spi_nand_page_is_free(row, &is_free);

    // check for error..
    if (SPI_NAND_RET_OK != ret) {
        shell_printf_line("Error when attempting to check if page is free: %d.", ret);
    }
    else {
        if (is_free) {
            shell_print_line("True.");
        }
        else {
            shell_print_line("False.");
        }
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