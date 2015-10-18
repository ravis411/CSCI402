/* testPageTable.c
 *	Simple program to test Lock syscalls
 */

#include "syscall.h"

#define arrySize 20

 int arry[arrySize];
 char welcomeString[] = "\nLock Syscall test...\n\n";


void function1(){
  PrintString("\nSuccessfully Forked Function1.\n", sizeof("\nSuccessfully Forked Function1.\n"));
  Exit(0);

}

void thread2(){
  PrintString( "\nSuccessfully Forked Thread2.\n", sizeof("\nSuccessfully Forked Thread2.\n"));
  Exit(0);
}

void thread3(){
  int i;
  PrintString("\nSucessfully Forked Thread3.\n", sizeof("\nSucessfully Forked Thread3.\n"));
  for(i = 0; i < 10; i++)
    PrintInt(i);
  Exit(0);
}

void thread4(){
  int i;
  /*char testString[] = "Thread4 Done.\n"; WHY CANT WE DO SOMETHING LIKE THIS!!! UGGHGH*/
  PrintString("\nSucessfully Forked Thread4.\n", sizeof("\nSucessfully Forked Thread4.\n"));
  for(i = 0; i < 10; i++)
    PrintInt(i);
  Write("\nThread4 Done.\n", sizeof("\nThread4 Done.\n"), ConsoleOutput);
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
  Release(lock1);

  DestroyLock(lock1);
	
	
	PrintString("\n", 1);
  Exit(0);
}