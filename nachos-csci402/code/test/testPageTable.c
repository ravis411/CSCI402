/* testPageTable.c
 *	Simple program to test pageTable
 */

#include "syscall.h"

 int arry[100];
 char testString[] = "This is a test.\n";

int main() {

  //init i
  for(int i = 0; i < 100; i++){
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


    Write(testString, sizeof("This is a test."), ConsoleOutput);

}
