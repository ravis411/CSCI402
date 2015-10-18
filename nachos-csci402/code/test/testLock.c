/* testPageTable.c
 *	Simple program to test Lock syscalls
 */

#include "syscall.h"

char welcomeString[] = "\nLock Syscall test...\n\n";
char string1[] = "String 1...\n";
char string2[] = "String 2...\n";
char string3[] = "String 3...\n";
char string4[] = "String 4...\n";
char string5[] = "String 5...\n";
char string6[] = "String 6...\n";

int lock1;
int done = 0;


void function1(){
  Acquire(lock1);
  PrintString(string1, sizeof(string1));
  Release(lock1);
  Acquire(lock1);
  PrintString(string3, sizeof(string3));
  Release(lock1);
  Acquire(lock1);
  PrintString(string5, sizeof(string5));
  Release(lock1);
  Exit(0);

}

void thread2(){
  Acquire(lock1);
  PrintString(string2, sizeof(string2));
  Release(lock1);
  Acquire(lock1);
  PrintString(string4, sizeof(string4));
  Release(lock1);
  Acquire(lock1);
  PrintString(string6, sizeof(string6));
  Release(lock1);
  done = 1;
  Exit(0);
}


int main() {
  int i;
  int lock1;
  int lock2;

  Write(welcomeString, sizeof(welcomeString), ConsoleOutput);

  lock1 = CreateLock();

  Acquire(lock1);
  PrintString("Lock Acquired.\n", sizeof("Lock Acquired.\n") );

  Fork(function1);
  Fork(thread2);


  Release(lock1);

  DestroyLock(lock1);
	
	
  while(done == 0){
    Yield();
  }

  PrintString("Test For Bad input expected output are 3 error messages.\n", 
      sizeof("Test For Bad input expected output are 3 error messages.\n"));
  Acquire(-1);
  Acquire(5);
  Release(3);


	PrintString("Done.\n", sizeof("Done.\n"));
  Exit(0);
}