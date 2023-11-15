/**
 * @file util.c
 * @date Sunday, December 06, 2015 at 01:06:28 AM EST
 * @author Brandon Perez (bmperez)
 * @author Jared Choi (jaewonch)
 *
 * This file contains miscalaneous utilities for out system.
 *
 * @bug No known bugs.
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <fcntl.h>              // Flags for open()
#include <sys/stat.h>           // Open() system call
#include <sys/types.h>          // Types for open()
#include <unistd.h>             // Read() and write()
#include <errno.h>              // Error codes

/*----------------------------------------------------------------------------
 * Command-Line Parsing Utilities
 *----------------------------------------------------------------------------*/

// Parses the arg string as an integer for the given option
int parse_int(char option, char *arg_str, int *data)
{
    return 0;
}

// Parses the arg string as a double for the given option
int parse_double(char option, char *arg_str, double *data)
{
    return 0;
}

int parse_resolution(char option, char *arg_str, int *height, int *width,
        int *depth)
{
    return 0;
}

/*----------------------------------------------------------------------------
 * File Operation Utilities
 *----------------------------------------------------------------------------*/

// Performs a robust read, reading out all bytes from the buffer
int robust_read(int fd, char *buf, int buf_size)
{
    return 0;
}

int robust_write(int fd, char *buf, int buf_size)
{
    return 0;
}

#ifdef __cplusplus
}
#endif
