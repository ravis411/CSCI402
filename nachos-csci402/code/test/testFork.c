/* testPageTable.c
 *	Simple program to test Fork syscall
 */

#include "syscall.h"

#define arrySize 20

 int arry[arrySize];
 char welcomeString[] = "\n\nFork Syscall test...\n\n";

int main() {

  int i;
	OpenFileId fd;
  int bytesread;
  char buf[20];


Write(welcomeString, sizeof(welcomeString), ConsoleOutput);

for (i = 0; i < arrySize; i++)
    arry[i] = i;

    Create("testfile", 8);
    fd = Open("testfile", 8);
    Write("testing a write\n", 16, fd );
    Close(fd);
    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);


    
	/*How to print out the array?*/
	for( i = 0; i < arrySize; i++){
		PrintInt(i);
		Write(" ", 1, ConsoleOutput);
	}




	Write("\n", 1, ConsoleOutput);
	Write("\n", 1, ConsoleOutput);
}

