#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

void *_sbrk(int incr)
{
    extern int end;
    static char *heap_end;
    char *prev_heap_end;

    if (!heap_end)
        heap_end = (char *)&end;

    prev_heap_end = heap_end;
    heap_end += incr;

    return prev_heap_end;
}

int _open(char *path, int flags, ...)
{
    return -1;
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

void _kill(int pid, int sig)
{
    return;
}

void _exit(int status)
{
    _kill(status, -1);
    while (1)
    {
    }
}

int _getpid(void)
{
    return -1;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    return 0;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    return 0;
}