/* testPageTable.c
 *	Simple program to test Condition syscalls
 */

#include "syscall.h"

char welcomeString[] = "\nCondition Syscall test...\n\n";
char string1[] = "String 1...\n";
char string2[] = "String 2...\n";
char string3[] = "String 3...\n";
char string4[] = "String 4...\n";
char string5[] = "String 5...\n";
char string6[] = "String 6...\n";

int lock1;
int condition1;
int done = 0;


void function1(){
  Acquire(lock1);
  
  PrintString(string1, sizeof(string1));
  
  Wait(condition1, lock1);

  PrintString(string3, sizeof(string3));
  
  Signal(condition1, lock1);
  Wait(condition1, lock1);

  PrintString(string5, sizeof(string5));

  Signal(condition1, lock1);
  Release(lock1);

  Exit(0);
}

void thread2(){
  Acquire(lock1);

  PrintString(string2, sizeof(string2));

  Signal(condition1, lock1);
  Wait(condition1, lock1);

  PrintString(string4, sizeof(string4));

  Signal(condition1, lock1);
  Wait(condition1, lock);
  
  PrintString(string6, sizeof(string6));

  Release(lock1);
  done = 1;
  Exit(0);
}


int main() {
  int i;

  Write(welcomeString, sizeof(welcomeString), ConsoleOutput);

  lock1 = CreateLock();
  condition1 = CreateCondition();


  Fork(function1);
  Yield();
  Acquire(lock1);
  Fork(thread2);
  Release(lock1);
	
	
  while(done == 0){
    Yield();
  }

  PrintString("Test For Bad input expected output are 3 error messages.\n", 
      sizeof("Test For Bad input expected output are 3 error messages.\n"));
  Wait(-1);
  Signal(5);
  Broadcast(3);


	PrintString("Done.\n", sizeof("Done.\n"));
  Exit(0);
}