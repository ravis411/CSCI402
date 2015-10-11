/* testPageTable.c
 *	Simple program to test pageTable
 */

#include "syscall.h"

#define arrySize 10

 int arry[arrySize];
 char testString[] = "This is a test.\n";

int main() {

  /*init i*/
  int i = 0;
  for(i = 0; i < arrySize; i++){
    arry[i] = i;
  }

  OpenFileId fd;
  int bytesread;
  char buf[20];

    Create("testfile", 8);
    fd = Open("testfile", 8);

    Write("testing a write\n", 16, fd );
    Close(fd);


    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);


    Write(testString, 16, ConsoleOutput);

}

