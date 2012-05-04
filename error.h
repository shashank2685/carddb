#ifndef __ERROR_H
#define __ERROR_H

#include <stdio.h>

/***************************************
 * ERROR CODES
 ***************************************/

extern int DbgPrintLevel;

#define ID_LEN 20 // 2^64 - 1 has 20 digits in it.

#define DBG_PRINT_ERR(Fmt) \
  printf ("\nError : "); \
  printf  Fmt ; \
  printf ("\n");


// Return Codes
#define TRUE 1
#define FALSE 0

#define READ_MODE 0
#define WRITE_MODE 1

#define BASE_ERROR 10
#define INSUFICIENT_ARGUMENTS  BASE_ERROR + 1
#define UNKNOWN_COMMAND BASE_ERROR + 2
#define INVALID_ID BASE_ERROR + 3
#define DB_OPEN_ERROR BASE_ERROR + 4
#define DB_NULL_ERROR BASE_ERROR + 5
#define LONG_ID_ERROR BASE_ERROR + 6
#define HANDLE_NULL_ERROR BASE_ERROR + 7
#define ELE_NOT_FOUND_ERROR BASE_ERROR + 8

#ifndef SQL_LITE
#define DB_CORRUPT_ERROR BASE_ERROR + 100 // This should never happen.
#define DUPLICATE_KEY_ERROR BASE_ERROR + 1
#endif // ifndef SQL_LITE

#ifdef SQL_LITE
#define BASE_SQL_ERROR 50
#define SQLITE_OPEN_ERROR BASE_SQL_ERROR + 1
#define CREATE_TABLE_ERROR BASE_SQL_ERROR + 2
#define INSERT_INTO_ERROR BASE_SQL_ERROR + 3
#define FIND_ERROR BASE_SQL_ERROR + 4
#define DELETE_ERROR BASE_SQL_ERROR + 5

#endif // ifdef SQL_LITE

#ifdef DBG

// Debug Levels
#define BASE_PRINT_LEVEL 20
#define DBG_TRACE BASE_PRINT_LEVEL + 1
#define DBG_WARN BASE_PRINT_LEVEL + 2
#define DBG_OFF BASE_PRINT_LEVEL + 3

#define DBG_PRINT(Level, Fmt) \
  if ( Level >= DbgPrintLevel ) { \
    printf("level = %d\n", Level); \
    printf Fmt ; \
    printf("\n"); \
  }

#else // DBG

#define DBG_TRACE 
#define DBG_WARN 
#define DBG_OFF 

#define DBG_PRINT(Level, Fmt)

#endif // ifdef DBG

#endif // ifndef __ERROR_H
