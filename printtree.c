#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

int indent = 0;

void traverse(FILE *fp)
{
  uint64_t id = 0ULL;
  long left = 0L;
  long right = 0L;
  long curPos = 0L;


  fread( &id, sizeof(id), 1, fp);
  fread( &left, sizeof(left), 1, fp);
  fread( &right, sizeof(right), 1, fp);

  if ( left != -1L ) {
    curPos = left;
    fseek(fp, left, SEEK_SET);
    indent++;
    traverse(fp);
    indent--;
    fseek(fp, curPos, SEEK_SET);
  }

  int temp = indent;
  while ( temp > 0) {
    printf("\t");
    temp--;
  }

  printf("N=%llu\n", id);
  
  if ( right != -1L ) {
    curPos = right;
    fseek(fp, right, SEEK_SET);
    indent++;
    traverse(fp);
    indent--;
    fseek(fp, curPos, SEEK_SET);
  }
  return;

}


int main( int argc, char *argv[])
{

  FILE *fp = fopen(argv[1], "r+b");

  if ( fp == NULL) {
    printf( "File cannot be opened\n");
    return 0;
  }

  traverse(fp);
}

