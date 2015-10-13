/* testPageTable.c
 *	Simple program to test Fork syscall
 */

#include "syscall.h"

#define arrySize 20

 int arry[arrySize];
 char welcomeString[] = "\n\nFork Syscall test...\n\n";


void function1(){
  Write("\nSuccessfully Forked Function1.\n", sizeof("\nSuccessfully Forked Function1.\n"), ConsoleOutput );
  Exit(0);

}

void thread2(){
  Write( "\nSuccessfully Forked Thread2.\n", sizeof("\nSuccessfully Forked Thread2.\n"), ConsoleOutput );
  Exit(0);
}

void thread3(){
  int i;
  Write("\nSucessfully Forked Thread3.\n", sizeof("\nSucessfully Forked Thread3.\n"), ConsoleOutput );
  for(i = 0; i < 10; i++)
    PrintInt(i);
  Exit(0);
}

void thread4(){
  int i;
  Write("\nSucessfully Forked Thread4.\n", sizeof("\nSucessfully Forked Thread4.\n"), ConsoleOutput );
  for(i = 0; i < 10; i++)
    PrintInt(i);
  char testString[] = "Thread4 Done.\n";
  Write(testString, sizeof(testString), ConsoleOutput);
  Exit(0);
}



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

  /*Now lets test fork...*/
  Fork(function1);
  Fork(thread3);
  Fork(thread2);
  Fork(thread4);
	
	Write("\n", 1, ConsoleOutput);
}

