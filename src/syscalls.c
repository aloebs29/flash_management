#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "modules/uart.h"

void *_sbrk(int incr)
{
    extern int end;
    static char *heap_end;
    char *prev_heap_end;

    if (!heap_end) heap_end = (char *)&end;

    prev_heap_end = heap_end;
    heap_end += incr;

    return prev_heap_end;
}

int _open(char *path, int flags, ...) { return -1; }

int _close(int file) { return -1; }

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) { return 1; }

int _lseek(int file, int ptr, int dir) { return 0; }

void _kill(int pid, int sig) { return; }

void _exit(int status)
{
    _kill(status, -1);
    while (1) {
    }
}

int _getpid(void) { return -1; }

int _read(int file, char *ptr, int len)
{
    if (STDIN_FILENO == file) {
        int i; // this needs to persist after the for loop
        for (i = 0; i < len; i++) {
            if (!_uart_try_getc(&ptr[i])) {
                break;
            }
        }
        return i;
    }
    else {
        // anything other than STDIN is invalid
        errno = EBADF;
        return -1;
    }
}

int _write(int file, char *ptr, int len)
{
    if ((STDOUT_FILENO == file) || (STDERR_FILENO == file)) {
        for (int i = 0; i < len; i++) {
            _uart_putc(ptr[i]);
        }
        return len;
    }
    else {
        // anything other than STDOUT or STDERR is invalid
        errno = EBADF;
        return -1;
    }
}
