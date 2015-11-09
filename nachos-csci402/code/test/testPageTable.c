/* testPageTable.c
 *	Simple program to test pageTable
 */

#include "syscall.h"

#define arrySize 15

 int arry[arrySize];
 char testString[] = "This is a test.\n";

int main() {

  int i;
	OpenFileId fd;
  int bytesread;
  char buf[20];
	char out[5];

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


    Write(testString, 16, ConsoleOutput);

	/*How to print out the array?*/
	for( i = 0; i < arrySize; i++){
		out[0] = arry[i] + '0';
		Write(out, 1, ConsoleOutput);
		Write(" ", 1, ConsoleOutput);
	}

	Write("\n", 1, ConsoleOutput);
/*Need to be able to do something like this...but how? To print out larger than single digit nums
	for( i = 0; i < arrySize; i++){
		Write("" + arry[i] + '0', sizeof("" + arry[i] + '0'), ConsoleOutput);
	}*/
	Write("\n", 1, ConsoleOutput);
}

