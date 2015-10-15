/* testPageTable.c
 *	Simple program to test Exec syscall
 */

#include "syscall.h"

#define arrySize 20

 int arry[arrySize];
 char welcomeString[] = "\nExec Syscall test...\n\n";
 char fileName[] = "../test/testExec";


void function1(){
  Write("\nSuccessfully Forked Function1.\n", sizeof("\nSuccessfully Forked Function1.\n"), ConsoleOutput );
  Exit(0);

}


int main() {

  int i;
	OpenFileId fd;
  int bytesread;
  char buf[20];

  Write(welcomeString, sizeof(welcomeString), ConsoleOutput);

  Write("About to call exec...\n", sizeof("About to call exec...\n"), ConsoleOutput);

  Exec(fileName);

	
Write("\n", 1, ConsoleOutput);
Write("\n", 1, ConsoleOutput);
}

