/* testPageTable.c
 *	Simple program to test Exec syscall
 */

#include "syscall.h"

 char welcomeString[] = "\nExec Syscall test...\n\n";
 char fileName[] = "../test/testFork";

int main() {

  PrintString(welcomeString, sizeof(welcomeString));

  Exec(fileName, sizeof(fileName));
  Exec(fileName, sizeof(fileName));

	
PrintString("\n", 1);
PrintString("\n", 1);
}

