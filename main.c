#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#ifdef SQL_LITE
#include "sqlite3.h"
#endif

#include "dbrec.h"
#include "error.h"

/*****************************
 * Debug level variable.
 * ***************************/
int DbgPrintLevel = DBG_OFF;


/****************************************
 * Function print_help
 * Purpose : print the usage message
 *
 * Return : void
 *
 ****************************************/
void print_help(char *progName)
{
  printf("\nUSAGE \n\t %s <db_file_name> <insert | delete | find> <id>\n", progName);
}

/***************************************
 *  Function check_valid_id
 *  Purpose checks the validity of input
 *
 *  id : string pointing to valid id
 *  
 *  Return : TRUE if the given id valid else FALSE
 *  ************************************/
int check_valid_id( char *id )
{
  int i = 0; // iterator.
  if ( ID_LEN < strlen(id) ) {
    DBG_PRINT(DBG_WARN, ("Length of Id is Greater Than 8 Bytes"))
    return FALSE;
  } // if ( ID_LEN < strlen(id) ) 

  for ( i = 0; i < strlen(id); i++) {

    if ( ! isdigit( id[i] ) ) {
      DBG_PRINT(DBG_WARN,("Invalid Character in ID"));
      return FALSE;
    } // if ( ! isdigit( id[i] ) )

  } // for ( i = 0; i < strlen(id); i++)

  return TRUE;

} // end of check_valid_id

/*****************************************
 * Function : extractId
 * Purpose convert the id from string to uint64_t format.
 *
 * id : In Parameter string containing the id.
 * indId : OUT Parameter variable which contains result after conversion.
 *
 * Return : success or error code.
 ******************************************/
int extractId( char *id , uint64_t *intId)
{
  char *endptr;
  *intId = strtoull(id, &endptr, 10);
  if ( *intId == ULLONG_MAX && errno == ERANGE) {
    return LONG_ID_ERROR;
  }

  return 0;
}

/***************************************
 * Function main
 * Purpose: Handle input arguments and call
 * input approiate functions to handle requested action.
 *
 * @argc : numer of arguments given in command line.
 * @argv : array containing the input arguments.
 *
 * Return : Success or error code.
 *
 * *************************************/
int main(int argc, char *argv[])
{

  char *id = argv[3];
  uint64_t intId = 0;
  int returnVal = 0;

  db_handle_t db_handle;

  db_handle.handle = NULL;
  db_handle.fileName = NULL;
  if ( argc != 4)
  {
    DBG_PRINT_ERR(("Insuficient Arguments"));
    print_help(argv[0]);
    return INSUFICIENT_ARGUMENTS;
  }

  // check id is valid.
  if ( ! check_valid_id( id ) ) {
    DBG_PRINT(DBG_WARN, ("Invalid Id"));
    return INVALID_ID;
  }

  if ( extractId(id, &intId) ) {
    DBG_PRINT(DBG_WARN, ("Invalid Id"));
    return INVALID_ID;
  }

  DBG_PRINT(DBG_TRACE, ("ID Check Was SUCCESSFUL"));

  if ( 0 == strcmp(argv[2], "insert") ) {
   
    db_handle.mode = WRITE_MODE; 
    returnVal = db_open( &db_handle, argv[1] );
    if ( 0 != returnVal )
    {
      DBG_PRINT(DBG_WARN, ("Unable to Open Database File\n"));
      return DB_OPEN_ERROR;    
    }
    
    returnVal = db_insert(&db_handle, intId);
    if ( returnVal != 0) {
      DBG_PRINT(DBG_WARN, ("Unable to insert in to the Database"));
      goto Error;
    }

  } else if ( 0 == strcmp(argv[2], "find")) {
    db_handle.mode = READ_MODE; 
    returnVal = db_open( &db_handle, argv[1] );
    if ( 0 != returnVal )
    {
      DBG_PRINT(DBG_WARN, ("Unable to Open Database File\n"));
      return DB_OPEN_ERROR;    
    }
    
    returnVal = db_find(&db_handle, intId);
    if ( returnVal != 0) {
      DBG_PRINT(DBG_WARN, ("Unable to search in the Database"));
      goto Error;
    }

  } else if ( 0 == strcmp(argv[2], "delete")) {
    db_handle.mode = WRITE_MODE; 
    returnVal = db_open( &db_handle, argv[1] );
    if ( 0 != returnVal )
    {
      DBG_PRINT(DBG_WARN, ("Unable to Open Database File\n"));
      return DB_OPEN_ERROR;    
    }
    
    returnVal = db_delete(&db_handle, intId);
    if ( returnVal != 0) {
      DBG_PRINT(DBG_WARN, ("Unable to delete from the Database"));
      goto Error;
    }
  } else {
    DBG_PRINT_ERR(("Unknow command"));
    print_help(argv[0]);
    return UNKNOWN_COMMAND;
  }

Error:
  db_close(&db_handle);
  return returnVal; 
}
