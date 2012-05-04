#ifndef __DBREC_H__
#define __DBREC_H__

#include <stdint.h> // uint64_t

/**********************************************************
PURPOSE:
		Open the file as the data base. 
		Insert one of more cecords in ot it.
		Find wether a card id exists in the database.
		Delete a card id in the database
		Close the databse.

TODO:  
	1. Define the structure for db_handle_t and 
	implement the given below functions.
            
*********************************************************/
typedef struct database_handle {

#ifdef SQL_LITE
  sqlite3 *handle;
#else
  FILE *handle;
#endif

  const char *fileName;
  int   mode; // 1 for write mode 0 for read mode.

} db_handle_t; 

/******************************************************
 * Function: db_open
 * Purpose: Initialize the date base from the file named 
 * filename
 *
 * @ db:          handle created for the data base
 * @ filename: file to be used as database
 *
 * Return: success or error code
******************************************************/
int db_open(db_handle_t * db, const char * filename);
/*****************************************************/


/******************************************************
 * Function: db_insert
 * Purpose: insert the id to the database
 *
 * @ db:          handle for the data base
 * @ cardid:    8 byte number to be insterted.
 *
 * Return: success or error code
******************************************************/
int db_insert(db_handle_t *db, uint64_t cardid);
/*****************************************************/


/******************************************************
 * Function: db_find
 * Purpose: search a card id in the database
 *
 * @ db:          handle for the data base
 * @ cardid:    8 byte number to be searched.
 *
 * Return: success or error code
******************************************************/
int db_find(db_handle_t *db, uint64_t cardid);
/*****************************************************/

/******************************************************
 * Function: db_delete
 * Purpose: delete the card id from the database
 *
 * @ db:          handle for the data base
 * @ cardid:    8 byte number to be searched.
 *
 * Return: success or error code
******************************************************/
int db_delete(db_handle_t *db, uint64_t cardid);
/*****************************************************/

/******************************************************
 * Function: db_close
 * Purpose: close the database
 * @ db:          handle for the data base
 *
 * Return: success or error code
******************************************************/
int db_close(db_handle_t *db);
/*****************************************************/


#endif /* __DBREC_H__ */
