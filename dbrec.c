#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SQL_LITE
#include "sqlite3.h"
#endif

#include "dbrec.h"
#include "error.h"

#define TABLE_NAME "'id_table'"
#define BUF_SIZE 1024

extern int DbgPrintLevel;

#ifndef SQL_LITE

long prevPos = -1L;
/***************************************************
 * Function traverse
 * Purpose Traverse form the root of the tree to node containing the required id.
 *         Duplicate ids are not allowed.
 *
 * fp : pointer to the file.
 * cardid : card id.
 * Return : succes or error code.
 **************************************************/
int traverse(FILE *fp, uint64_t cardid)
{
  uint64_t id = 0ULL;
  long left = 0L;
  long right = 0L;
  int leftReturnVal = 0;
  int rightReturnVal = 0;

  fread(&id, sizeof(id), 1, fp);
  fread(&left, sizeof(left), 1, fp);
  fread(&right, sizeof(right), 1, fp);

  if ( id == cardid ) {
    // current implementation do not allow duplicates.
    DBG_PRINT(DBG_WARN, ("Duplicate Entry %llu\n", cardid));
    return DUPLICATE_KEY_ERROR;
  }

  if ( cardid < id) {
    if ( left != -1L ) {
      prevPos = ftell(fp) - (long)( sizeof(uint64_t) + (2 * sizeof(long)));
      fseek(fp, left, SEEK_SET);
      leftReturnVal = traverse(fp, cardid);    
    } else {
      // found the approiate position.
      return 0;
    }
  }

  if ( cardid > id ) {
    if ( right != -1L ) {
      prevPos = ftell(fp) - (long)( sizeof(uint64_t) + (2 * sizeof(long)));
      fseek(fp, right, SEEK_SET);
      rightReturnVal = traverse(fp, cardid);    
    } else { 
      // found the approiate position.
      return 0;
    }
  }

  return leftReturnVal | rightReturnVal;
}

#endif // ifndef SQL_LITE
int db_open(db_handle_t *db, const char * filename)
{
  FILE *fp = NULL;
  int returnVal = 0;

  if ( NULL == filename )
  {
    DBG_PRINT(DBG_WARN, ("Null Passed to Filename"));
    returnVal = DB_NULL_ERROR;
    goto OPEN_ERR;
  }


#ifndef SQL_LITE
  if ( db->mode == WRITE_MODE ) {
    fp = fopen(filename, "r+b");
    // Tried to open in read and write mode.
    if ( NULL == fp ) {
      // file may not exist. Try it in write mode.
      fp = fopen(filename, "w+b");
    }
  }


  if ( db->mode == READ_MODE )
    fp = fopen(filename, "rb" );

  if ( NULL == fp )
  {
    DBG_PRINT(DBG_WARN, ("Unable to Open File"));
    returnVal = DB_OPEN_ERROR;
    goto OPEN_ERR;
  }

  db->handle = fp;

#endif

#ifdef SQL_LITE
  char *errMsg = NULL;
  char sql[BUF_SIZE];
  memset(sql, 0, BUF_SIZE);

  returnVal = sqlite3_open(filename, &db->handle);
  if ( returnVal )
  {
    DBG_PRINT(DBG_WARN, ("Cannot Open Database %s\n", sqlite3_errmsg(db->handle)));
    returnVal = SQLITE_OPEN_ERROR;
    db->handle = NULL;
    goto OPEN_ERR;
  }

  // check if table exitst. If not create one.
#ifdef ALLOW_DUPLICATES
  sprintf(sql, "CREATE TABLE IF NOT EXISTS %s ( id int8 )", TABLE_NAME);
#else
  sprintf(sql, "CREATE TABLE IF NOT EXISTS %s ( id int8 primary key )", TABLE_NAME);
#endif
  returnVal = sqlite3_exec(db->handle, sql, NULL, 0, &errMsg);

  if ( returnVal ) {
    DBG_PRINT(DBG_WARN, ("Error While Creating Table %s\n", errMsg));
    sqlite3_free(errMsg);
    returnVal = CREATE_TABLE_ERROR;
    goto OPEN_ERR;
  }

#endif
  db->fileName = filename;
OPEN_ERR:
  return returnVal;
}

int db_insert(db_handle_t *db, uint64_t id)
{
  int returnVal = 0;

  if ( NULL == db ) {
    // This should not happen. So warn and return error
    DBG_PRINT(DBG_WARN, ("Db handle is NULL"));
    returnVal = HANDLE_NULL_ERROR;
    goto INSERT_ERR;
  }

#ifdef SQL_LITE
  char sql[BUF_SIZE];
  char *errMsg;

  memset(sql, 0, BUF_SIZE);
  sprintf(sql, "INSERT INTO %s ( id ) VALUES ( %llu )", TABLE_NAME, id);

  returnVal = sqlite3_exec(db->handle, sql, NULL, 0, &errMsg); 

  if ( returnVal ) {
    DBG_PRINT(DBG_WARN, ("Error While Inserting id = %lld. %s", id, errMsg));
    sqlite3_free(errMsg);
    returnVal = INSERT_INTO_ERROR;
    goto INSERT_ERR;
  }

#else
  // Start From Begining.
  long left = -1L;
  long right = -1L;

  fseek(db->handle, 0L, SEEK_END);
  if ( 0L == ftell(db->handle )) {

    // This is the first entry.
    returnVal = fwrite(&id, sizeof(id), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto INSERT_ERR;
    }

    returnVal = fwrite(&left, sizeof(left), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto INSERT_ERR;
    }

    returnVal = fwrite(&right, sizeof(right), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto INSERT_ERR;
    }
    fflush(db->handle);
    returnVal = 0;
    goto INSERT_ERR;
  }
  // Go to Begining.
  fseek(db->handle, 0L, SEEK_SET);
  returnVal = traverse(db->handle, id);

  if ( returnVal != 0 ) 
    goto INSERT_ERR;

  long totLen = sizeof(id) + sizeof(left) + sizeof(right);
  totLen = -totLen;

  uint64_t curid = 0ULL;
  // go back totLen bytes in file
  fseek(db->handle, totLen, SEEK_CUR);
  long curPos = ftell(db->handle);

  fread(&curid, sizeof(id), 1, db->handle);

  totLen = -totLen;
  if ( id < curid ) {
    curPos += sizeof(curid);
  } else {
    curPos += (sizeof(curid) + sizeof(left));
  }

  // Go to the end to insert new value 
  fseek(db->handle, 0L, SEEK_END);
  returnVal = fwrite(&id, sizeof(id), 1, db->handle);
  if ( 1 != returnVal )
  {
    DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
    returnVal = DB_CORRUPT_ERROR;
    goto INSERT_ERR;
  }

  returnVal = fwrite(&left, sizeof(left), 1, db->handle);
  if ( 1 != returnVal )
  {
    DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
    returnVal = DB_CORRUPT_ERROR;
    goto INSERT_ERR;
  }

  returnVal = fwrite(&left, sizeof(left), 1, db->handle);
  if ( 1 != returnVal )
  {
    DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
    returnVal = DB_CORRUPT_ERROR;
    goto INSERT_ERR;
  }
  fflush(db->handle);

  long curPos2 = ftell(db->handle);
  curPos2 -= totLen;
  // Store new inserted postion.
  fseek(db->handle, curPos, SEEK_SET);
  returnVal = fwrite(&curPos2, sizeof(curPos2), 1, db->handle);
  if ( 1 != returnVal )
  {
    DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
    returnVal = DB_CORRUPT_ERROR;
    goto INSERT_ERR;
  }

  return 0;
#endif

INSERT_ERR: 
  return returnVal;

} // end of db_insert

/***************************************************
 * Function : callback
 * Purpose  : counts the number of results returned by the sql statement.
 *            Since this callback is called for every result row in sql statement.
 *            counter is incremeted by one for each row in the result.
 *
 * counter : only counter is of interest here. It keeps the final
 *           count of number rows in the sql query result.
 * Retrun : 0 every time
 * ***************************************************/
static int callback(void *counter, int argc, char **argv, char **azColName){

  * ((int *)counter) = * ((int *) counter) + 1;
  return 0;
}

int db_find( db_handle_t *db, uint64_t cardid)
{

  int returnVal = 0;
  if ( NULL == db) {
    // This should not happen. So warn and return error
    DBG_PRINT(DBG_WARN, ("Db handle is NULL"));
    returnVal = HANDLE_NULL_ERROR;
    goto FIND_ERR;
  }

#ifdef SQL_LITE
  char sql[BUF_SIZE];
  char *errMsg;
  int  counter = 0;

  memset(sql, 0, BUF_SIZE);
  sprintf(sql, "SELECT * FROM %s WHERE id = %llu", TABLE_NAME, cardid);
  returnVal = sqlite3_exec(db->handle, sql, callback, (void *)&counter, &errMsg); 

  if ( returnVal ) {
    DBG_PRINT(DBG_WARN, ("Error While Searching id = %llu. %s", cardid, errMsg));
    sqlite3_free(errMsg);
    returnVal = FIND_ERROR;
    goto FIND_ERR;
  }

  if ( counter == 0 )
    returnVal = 1;
  else
    returnVal = 0;
#else

  fseek(db->handle, 0L, SEEK_END);
  if ( 0L == ftell(db->handle) ) {
    DBG_PRINT(DBG_WARN, ("Empty File"));
    returnVal = 1;
    goto FIND_ERR;
  }

  fseek(db->handle, 0L, SEEK_SET);
  returnVal = traverse(db->handle, cardid);
  if ( returnVal == DUPLICATE_KEY_ERROR ) {
    returnVal = 0;
  } else {
    returnVal = 1;
  }

#endif // SQL_LITE

FIND_ERR:
  return returnVal;
} // end of db_find

int db_delete(db_handle_t *db, uint64_t cardid)
{
  int returnVal = 0;
  if ( NULL == db) {

    // This should not happen. So warn and return error
    DBG_PRINT(DBG_WARN, ("Db handle is NULL"));
    returnVal = HANDLE_NULL_ERROR;
    goto DEL_ERROR;

  } // if ( NULL == db )

#ifdef SQL_LITE
  char sql[BUF_SIZE];
  char *errMsg;
  int  counter = 0;

  memset(sql, 0, BUF_SIZE);
#ifdef ALLOW_DUPLICATES
  sprintf(sql, "SELECT * FROM %s WHERE id = %llu LIMIT 1", TABLE_NAME, cardid);
#else
  sprintf(sql, "SELECT * FROM %s WHERE id = %llu", TABLE_NAME, cardid);
#endif
  returnVal = sqlite3_exec(db->handle, sql, callback, (void *)&counter, &errMsg); 

  if ( returnVal ) {

    DBG_PRINT(DBG_WARN, ("Error While Searching id = %lld. %s", cardid, errMsg));
    sqlite3_free(errMsg);
    returnVal = FIND_ERROR;
    goto DEL_ERROR;

  } // if ( returnVal )

  if ( counter == 0 ) {

    DBG_PRINT(DBG_WARN, ("Card id = %llu Not Found in Our Database", cardid));
    returnVal = ELE_NOT_FOUND_ERROR;
    goto DEL_ERROR;

  } else {

    memset(sql, 0, BUF_SIZE);
    sprintf(sql, "DELETE FROM %s WHERE id = %llu LIMIT 1", TABLE_NAME, cardid);
    returnVal = sqlite3_exec(db->handle, sql, NULL, 0, &errMsg); 

    if ( returnVal ) {
      DBG_PRINT(DBG_WARN, ("Error While Deleting id = %llu. %s", cardid, errMsg));
      sqlite3_free(errMsg);
      returnVal = DELETE_ERROR;
      goto DEL_ERROR;
    } // if ( returnVal )

  } // if ( counter == 0)

#else

  returnVal = db_find(db, cardid);
  if ( returnVal != 0 ) {
    DBG_PRINT(DBG_WARN, ("Card id %llu Do Not Exists in Our Database"));
    return ELE_NOT_FOUND_ERROR;
  }

  uint64_t id = 0ULL;
  uint64_t prevId = 0ULL;
  long left = -1L;
  long right = -1L;
  long curPos = -1L;

  int totlen = sizeof(id) + sizeof(left) + sizeof(right);

  fseek(db->handle, (long)-totlen, SEEK_CUR); // Move to the begining of record.
  returnVal = fread(&id, sizeof(id), 1, db->handle);
  returnVal = fread(&left, sizeof(left), 1, db->handle);
  returnVal = fread(&right, sizeof(right), 1, db->handle);


  if ( -1L == left && -1L == right) {
    // Node is leaf.
    curPos = ftell(db->handle) - ((long) (sizeof(uint64_t) + 2 * sizeof(long)));
    if ( 0L == curPos ) {
      // only one node in the tree. Truncate the file.
      fclose(db->handle);
      db->handle = fopen(db->fileName, "wb"); // reopen in write mode. It will truncate it.
      returnVal = 0;
      goto DEL_ERROR;
    }

    // Delete the link 
    fseek(db->handle, prevPos, SEEK_SET);
    fread(&prevId, sizeof(prevId), 1, db->handle);

    if ( prevId < id ) {
      fseek(db->handle, prevPos + sizeof(left) + sizeof(prevId), SEEK_SET);
      returnVal = fwrite(&right, sizeof(right), 1, db->handle);
    } else {
      returnVal = fwrite(&left, sizeof(left), 1, db->handle);
    }

    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      perror(NULL);
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }

    fflush(db->handle);
    // Delete the node.
    fseek(db->handle, curPos, SEEK_SET);
    id = -1ULL;
    returnVal = fwrite(&id, sizeof(id), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }
    fflush(db->handle);
    return 0;
  }

  if ( -1L == left || -1L == right ) {
    //Node with one child
    if ( -1L == left ) {
      curPos = ftell(db->handle) - ((long) (sizeof(uint64_t) + 2 * sizeof(long)));
      fseek(db->handle, right, SEEK_SET);
      fread(&id, sizeof(id), 1, db->handle);
      fread(&left, sizeof(left), 1, db->handle);
      fread(&right, sizeof(right), 1, db->handle);

      fseek(db->handle, curPos, SEEK_SET);
      returnVal = fwrite(&id, sizeof(id), 1, db->handle);
      if ( 1 != returnVal )
      {
        DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
        returnVal = DB_CORRUPT_ERROR;
        goto DEL_ERROR;
      }
      returnVal = fwrite(&left, sizeof(left), 1, db->handle);
      if ( 1 != returnVal )
      {
        DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
        returnVal = DB_CORRUPT_ERROR;
        goto DEL_ERROR;
      }
      returnVal = fwrite(&right, sizeof(right), 1, db->handle);
      if ( 1 != returnVal )
      {
        DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
        returnVal = DB_CORRUPT_ERROR;
        goto DEL_ERROR;
      }
      fflush(db->handle);
      return 0; 
    } // if ( -1L == left )
    if ( -1L == right ) {
      curPos = ftell(db->handle) - ((long) (sizeof(uint64_t) + 2 * sizeof(long)));
      fseek(db->handle, left, SEEK_SET);
      fread(&id, sizeof(id), 1, db->handle);
      fread(&left, sizeof(left), 1, db->handle);
      fread(&right, sizeof(right), 1, db->handle);

      fseek(db->handle, curPos, SEEK_SET);
      returnVal = fwrite(&id, sizeof(id), 1, db->handle);
      if ( 1 != returnVal )
      {
        DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
        returnVal = DB_CORRUPT_ERROR;
        goto DEL_ERROR;
      }
      returnVal = fwrite(&left, sizeof(left), 1, db->handle);
      if ( 1 != returnVal )
      {
        DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
        returnVal = DB_CORRUPT_ERROR;
        goto DEL_ERROR;
      }
      returnVal = fwrite(&right, sizeof(right), 1, db->handle);
      if ( 1 != returnVal )
      {
        DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
        returnVal = DB_CORRUPT_ERROR;
        goto DEL_ERROR;
      }
      fflush(db->handle);
      return 0; 
    } // if ( -1L == left )

  } // if ( -1L == left || -1L == right )

  // go to the left child and keep on going right untill you see an 
  // node with no right child.
  curPos = ftell(db->handle) - ((long) (sizeof(uint64_t) + 2 * sizeof(long)));
  long parentPos = curPos;
  // go to left.
  fseek(db->handle, left, SEEK_SET); 
  long temp = -1L;
  while ( 1 ) {
    // Keep on going right.
    fread(&id, sizeof(id), 1, db->handle);
    fread(&left, sizeof(left), 1, db->handle);
    fread(&right, sizeof(right), 1, db->handle);
    temp = ftell(db->handle) - ((long) sizeof(uint64_t) + 2 * sizeof(long));
    if ( -1L == right )
      break;
    parentPos = temp;
    fseek(db->handle, right, SEEK_SET);
  }

  long deletedPos = ftell(db->handle) - ((long) sizeof(uint64_t) + 2 * sizeof(long));
  uint64_t replacementValue = id;

  // delete the node.
  if ( left == -1L && right == -1L ) {
    fseek(db->handle, deletedPos, SEEK_SET);
    fread(&id, sizeof(id), 1, db->handle);
    fseek(db->handle, parentPos, SEEK_SET);
    uint64_t parentId = -1ULL;
    fread(&parentId, sizeof(parentId), 1, db->handle);
    if ( parentId > id ) {
      fwrite(&left, sizeof(left), 1, db->handle);
    } else {
      fseek(db->handle, sizeof(left), SEEK_CUR);
      fwrite(&right, sizeof(right), 1, db->handle);
    }
    fseek(db->handle, deletedPos, SEEK_SET);
    id = -1ULL;
    returnVal = fwrite( &id, sizeof(id), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }
    fflush(db->handle);
    fseek(db->handle, (long)(temp + sizeof(id) + sizeof(left)), SEEK_SET);
    right = -1L;
    returnVal = fwrite(&right, sizeof(right), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }
    fflush(db->handle);
    // Replace the node.
    fseek(db->handle, curPos, SEEK_SET);
    returnVal = fwrite(&replacementValue, sizeof(replacementValue), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }
    fflush(db->handle); 
    return 0;
  } else {
    fseek(db->handle, curPos, SEEK_SET);
    returnVal = fwrite(&replacementValue, sizeof(replacementValue), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }

    fflush(db->handle); 
    fseek(db->handle, left, SEEK_SET);
    fread(&id, sizeof(id), 1, db->handle);
    fread(&left, sizeof(left), 1, db->handle);
    fread(&right, sizeof(right), 1, db->handle);

    fseek(db->handle, deletedPos, SEEK_SET);
    returnVal = fwrite(&id, sizeof(id), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }
    returnVal = fwrite(&left, sizeof(left), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }
    returnVal = fwrite(&right, sizeof(right), 1, db->handle);
    if ( 1 != returnVal )
    {
      DBG_PRINT_ERR(("Something bad Happened\n Database Corrupted. Sorry !!"));
      returnVal = DB_CORRUPT_ERROR;
      goto DEL_ERROR;
    }
    fflush(db->handle);
    return 0;
  }
#endif // SQL_LITE 
DEL_ERROR:
  return returnVal;
}

int db_close( db_handle_t *db)
{
  if ( db->handle != NULL)
#ifdef SQL_LITE
    sqlite3_close(db->handle);
#else
  fclose(db->handle);
#endif

  return 0;
}
