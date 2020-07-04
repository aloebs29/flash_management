/**
 * @file		shell.c
 * @author		Andrew Loebs
 * @brief		Implementation file of the shell module
 * 	
 */

#include "shell.h"

#include <stdio.h>
#include <stdbool.h>

// defines
#define RECEIVE_BUFFER_LEN          128 // max command + args length supported

// private function prototypes
static void receive_buffer_reset(void);
static bool try_get_char(char * out);
static void echo(char c);

// private variables
static char receive_buffer[RECEIVE_BUFFER_LEN];
static size_t receive_index;

// public function definitions
void shell_init(void)
{
    receive_buffer_reset();
}

void shell_tick(void)
{
    char c;
    if (try_get_char(&c))
    {
        // for now, just echo
        echo(c);
        // TODO: build CLI with receive buffer
    }
}

// private function definitions
static void receive_buffer_reset(void)
{
    receive_buffer[0] = 0;
    receive_index = 0;
}

static bool try_get_char(char * out)
{
    // To get around getchar() blocking, the _read() syscall returns
    // EOF for stdin whenever the UART rx is empty. Because of this,
    // we must check getchar() for EOF so that we know if we have a new
    // character, and rewind stdin when we do not.
    char c = getchar();
    if ((char)EOF == c) 
    {
        rewind(stdin);
        return false;
    }
    else
    {
        *out = c;
        return true;
    }
}

static void echo(char c)
{
    // handle newline
    if ('\r' == c) 
    {
        putchar('\r');
        putchar('\n');
    }
    // handle backspace
    else if ('\b' == c) 
    {
        putchar('\b');
        putchar(' ');
        putchar('\b');
    }
    // else, just echo
    else
    {
        putchar(c);
    }
}