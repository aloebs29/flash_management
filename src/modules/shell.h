/**
 * @file		shell.h
 * @author		Andrew Loebs
 * @brief		Header file of the shell module
 * 
 * Provides functionality for a virtual shell & CLI.
 * 	
 */

#ifndef __SHELL_H
#define __SHELL_H

/// @brief Initializes the shell
void shell_init(void);

/// @brief Gives processing time to the shell
void shell_tick(void);

/// @brief Writes a string + newline to the shell
void shell_print_line(const char * string);

/// @brief Writes formatted output + newline to the shell
void shell_printf_line(const char * format, ...);

#endif // __SHELL_H
