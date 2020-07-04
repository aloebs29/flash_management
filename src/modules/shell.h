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

#endif // __SHELL_H
