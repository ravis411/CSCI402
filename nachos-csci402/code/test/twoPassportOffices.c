/* twoPassportOffices.c
 *	Simple program tto test multiple passportOffices
 */

#include "syscall.h"

 char welcomeString[] = "\nStarting 2 passportOffce.c...\n\n\n";
 char fileName[] = "../test/passportOffce";

int main() {

  PrintString(welcomeString, sizeof(welcomeString));

  Exec(fileName, sizeof(fileName));
  Exec(fileName, sizeof(fileName));

	
Exit(0);
}

