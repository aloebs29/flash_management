/**
 * @file		shell_cmd.h
 * @author		Andrew Loebs
 * @brief		Header file of the shell command module
 *
 * Provides command processing for the shell.
 *
 */

#ifndef __SHELL_CMD_H
#define __SHELL_CMD_H

#include <stddef.h>

/// @brief Shell command handler delegate
typedef void (*shell_cmd_handler_t)(int argc, char *argv[]);

/// @brief Parses the command buffer into argv/argc and calls the relevant handler function
void shell_cmd_process(char *buff, size_t len);

#endif // __SHELL_CMD_H
