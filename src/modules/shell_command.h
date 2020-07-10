/**
 * @file		shell_command.h
 * @author		Andrew Loebs
 * @brief		Header file of the shell command module
 * 
 * Provides command processing for the shell.
 * 	
 */

#ifndef __SHELL_COMMAND_H
#define __SHELL_COMMAND_H

#include <stddef.h>

/// @brief Shell command handler delegate
typedef void (*shell_command_handler_t)(int argc, char * argv[]);

/// @brief Parses the command buffer into argv/argc and calls the relevant handler function
void shell_command_process(char * buff, size_t len);

#endif // __SHELL_COMMAND_H
