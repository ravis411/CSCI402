/* testPageTable.c
 *	Simple program to test Exec syscall
 */

#include "syscall.h"

 char welcomeString[] = "\nExec Syscall test...\n\n";
 char fileName[] = "../test/testFork";

int main() {

  Write(welcomeString, sizeof(welcomeString), ConsoleOutput);

  Exec(fileName, sizeof(fileName));
  Exec(fileName, sizeof(fileName));

	
Write("\n", 1, ConsoleOutput);
Write("\n", 1, ConsoleOutput);
}

