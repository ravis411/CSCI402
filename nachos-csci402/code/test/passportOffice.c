/* passportOffice.c
 *	Simple program to test Condition syscalls
 */

#include "syscall.h"

#define CLERKCOUNT  2
#define CUSTOMERCOUNT 3
#define SENATORCOUNT  0

#define MAXCLERKS 5
#define MAXCUSTOMERS 50
#define MAXSENATORS 10

int THEEND = 0;

#define AVAILABLE 0
#define SIGNALEDCUSTOMER 1
#define BUSY 2
#define ONBREAK 3

char CUSTOMERTEXT[] = "Customer";
char SENATORTEXT[] = "Senator";

/***********
* Locks
*********/
int applicationClerkLineLock;
int pictureClerkLineLock;
int passportClerkLineLock;
int cashierLineLock;
int managerLock;

int printLock;  /*For using the PrintSyscalls*/

int applicationClerkLock[MAXCLERKS];
int pictureClerkLock[MAXCLERKS];
int passportClerkLock[MAXCLERKS];
int cashierLock[MAXCLERKS];

/***************
* CVs
**********/
int passportOfficeOutsideLineCV; /* Outside line for when senators are present */
int senatorLineCV;

int applicationClerkLineCV[MAXCLERKS];
int applicationClerkBribeLineCV[MAXCLERKS];  /* //applicationClerk CVs */
int applicationClerkCV[MAXCLERKS];
int applicationClerkBreakCV; /*To keep track of clerks on break */

int pictureClerkLineCV[MAXCLERKS];
int pictureClerkBribeLineCV[MAXCLERKS];  /*pictureClerk CVs*/
int pictureClerkCV[MAXCLERKS];
int pictureClerkBreakCV;

int passportClerkLineCV[MAXCLERKS];
int passportClerkBribeLineCV[MAXCLERKS]; /*passportClerk CVs*/
int passportClerkCV[MAXCLERKS];
int passportClerkBreakCV;

int cashierLineCV[MAXCLERKS];
int cashierBribeLineCV[MAXCLERKS]; /* //passportClerk CVs */
int cashierCV[MAXCLERKS];
int cashierBreakCV;

/************
* States
*********/
int applicationClerkState[MAXCLERKS]; /* //applicationClerkState */
int pictureClerkState[MAXCLERKS];  /* //applicationClerkState */
int passportClerkState[MAXCLERKS];  /* //applicationClerkState */
int cashierState[MAXCLERKS];  /* //applicationClerkState */


/***********
* Line Counts
************/
int applicationClerkLineCount[MAXCLERKS];     /* //applicationClerkLineCount */
int applicationClerkBribeLineCount[MAXCLERKS];    /* //applicationClerkBribeLineCount */
int pictureClerkLineCount[MAXCLERKS];     /* //pictureClerkLineCount */
int pictureClerkBribeLineCount[MAXCLERKS];    /* //pictureClerkBribeLineCount */
int passportClerkLineCount[MAXCLERKS];      /* //passportClerkLineCount */
int passportClerkBribeLineCount[MAXCLERKS];   /* //passportClerkBribeLineCount */
int cashierLineCount[MAXCLERKS];      /* //cashierLineCount */
int cashierBribeLineCount[MAXCLERKS];   /* //cashierBribeLineCount */

/*******************
* Shared Data
******************/
int applicationClerkSharedData[MAXCLERKS];  /* //This can be used by the customer to pass SSN */
int pictureClerkSharedDataSSN[MAXCLERKS]; /* //This can be used by the customer to pass SSN */
int pictureClerkSharedDataPicture[MAXCLERKS]; /* // This can be used by the customer to pass acceptance of the picture */
int passportClerkSharedDataSSN[MAXCLERKS]; /* //This can be used by the customer to pass SSN */

int applicationCompletion[MAXCUSTOMERS + MAXSENATORS]; /* //Used by passportCerkto verify that application has been completed */
int pictureCompletion[MAXCUSTOMERS + MAXSENATORS]; /* //Used by passportClerk to verify that picture has beeen completed */
int passportCompletion[MAXCUSTOMERS + MAXSENATORS]; /* // Used by cashier to verify that the passport is complete */
int passportPunishment[MAXCUSTOMERS + MAXSENATORS]; /* //Used by passportClerk to punish bad people. */
int cashierSharedDataSSN[MAXCUSTOMERS + MAXSENATORS]; /* //This can be used by the customer to pass SSN */
int cashierRejection[MAXCUSTOMERS + MAXSENATORS]; /* //Used by the cashier to reject customers. */
int doneCompletely[MAXCUSTOMERS + MAXSENATORS]; /* //Used by customer to tell when done. */



int customersPresentCount = 0;/*For telling the manager we're in the office*/
int senatorPresentCount = 0;
int checkedOutCount = 0;  /*For the manager to put everyone to sleep when the customers have all finished*/
int senatorLineCount = 0; /*For counting the sentors.//They wait in a private line for the manager while waiting for customers to leave.*/
int passportOfficeOutsideLineCount = 0;
int senatorSafeToEnter = 0; /*To tell senators when it is safe to enter*/
int senatorPresentWaitOutSide = 0;/*Set by the manager to tell customers when a senator is present...*/


/*********************
*
* End Variables
*
************************************************/


/*****************************
* Broken Utility Functions...
***************************/

/*******************
 End Utility Functions
********************/



 /********************************************************************************
 *********************************************************************************
 *********************************************************************************
 *
 *  Customer
 *
 *********************************************************************************
 *********************************************************************************/

/*Wait outside or something there's a Senator present*/
void customerSenatorPresentWaitOutside(int SSN){
  Acquire(printLock);
  PrintString("Customer ", sizeof("Customer ")); 
  PrintInt(SSN);
  PrintString(" is going outside the PassportOffice because there is a Senator present.\n", 
    sizeof(" is going outside the PassportOffice because there is a Senator present.\n"));
  Release(printLock);

  /*Go outside.*/
  customersPresentCount--;
  passportOfficeOutsideLineCount++;
  Wait(passportOfficeOutsideLineCV, managerLock);
  /*Can go back inside now.*/
  passportOfficeOutsideLineCount--;
  customersPresentCount++;
}

/* Checks if a senator is present. Then goes outside if there is.*/
int customerCheckSenator(int SSN){
  int present;
  Acquire(managerLock);
  present = senatorPresentWaitOutSide;

  if(present)
    customerSenatorPresentWaitOutside(SSN);

  Release(managerLock);
  return present;
}

void customerCheckIn(int SSN){
  Acquire(managerLock);
  customersPresentCount++;
  Release(managerLock);
}

/*To tell the manager they did a great job and let him know we're done.*/
void customerCheckOut(int SSN){
  Acquire(managerLock);
  customersPresentCount--;
  checkedOutCount++;
  Release(managerLock);
  Acquire(printLock);
  PrintString("Customer ", sizeof("Customer ") );
  PrintInt(SSN);
  PrintString(" is leaving the Passport Office.\n", sizeof(" is leaving the Passport Office.\n"));
  Release(printLock);
  Exit(0);
}


void Customer(){
  int appClerkDone = 0;
  int pictureClerkDone = 0;
  int passportClerkDone = 0;
  int cashierDone = 0;
  int SSN = -1;
  int myLine = -1;
  int money = 700; /*(rand()%4)*500 + 100;*/
  int appClerkFirst = 0; /*rand() % 2;*/

  customerCheckIn(SSN);

  customerCheckOut(SSN);

  Exit(0);
}/*End Customer*/























/******************************************************
*******************************************************
*
* Manager
*
*******************************************************/

/*This will put the clerks and the manager to sleep so everyone can go to sleep and nachos can clean up*/
void checkEndOfDay(){
  Acquire(managerLock);

  if (checkedOutCount == (CUSTOMERCOUNT + SENATORCOUNT)){
    /*DEBUG('s', "DEBUG: MANAGER: END OF DAY!\n");
    All the customers are gone
    Lets all go to sleep*/
    THEEND = 1;
    Release(managerLock);

  /*currentThread->Finish();*/
    Exit(0);
  }
  Release(managerLock);
}

/* managerCheckandWakeupCLERK
* checks if a line has more than 3 customers... 
* if so, signals a clerk on break
* Returns true if there was asleeping clerk and needed to wake one up*/
int managerCheckandWakeupCLERK(int managerCWCLineLock, int* managerCWClineCount, int* managerCWCState, int managerCWCBreakCV, int managerCWCount){
  int wakeUp = 0;/*should we wake up a clerk?*/
  int asleep = 0;/*is any clerk asleep?*/
  int i;
  Acquire(managerCWCLineLock);
  for(i = 0; i < managerCWCount; i++){
    if(managerCWCState[i] == ONBREAK)
      asleep = 1;
    if(managerCWClineCount[i] > 3)
      wakeUp = 1;
  }
  if(wakeUp && asleep){Signal(managerCWCBreakCV, managerCWCLineLock);}
  Release(managerCWCLineLock);
  return asleep && wakeUp;
}

/*managerCheckandWakupClerks()
* Checks all types of clerks for lines longer than 3 and wakes up a sleaping clerk if there is one*/
void managerCheckandWakupClerks(){
  /*Check Application Clerks*/
  if(managerCheckandWakeupCLERK(applicationClerkLineLock, applicationClerkLineCount, applicationClerkState, applicationClerkBreakCV, CLERKCOUNT)){
    Acquire(printLock);
    PrintString("Manager has woken up an ApplicationClerk.\n", sizeof("Manager has woken up an ApplicationClerk.\n"));
    Release(printLock);
  }

  /*Check Picture Clerks*/
  if(managerCheckandWakeupCLERK(pictureClerkLineLock, pictureClerkLineCount, pictureClerkState, pictureClerkBreakCV, CLERKCOUNT)){
    Acquire(printLock);
    PrintString("Manager has woken up a PictureClerk.\n", sizeof("Manager has woken up a PictureClerk.\n") );
    Release(printLock);
  }
  
  /*Check Passport Clerks*/
  if(managerCheckandWakeupCLERK(passportClerkLineLock, passportClerkLineCount, passportClerkState, passportClerkBreakCV, CLERKCOUNT)){
    Acquire(printLock);
    PrintString("Manager has woken up a PassportClerk.\n", sizeof("Manager has woken up a PassportClerk.\n") );
    Release(printLock);
  }

  /*Check Cashiers*/
  if(managerCheckandWakeupCLERK(cashierLineLock, cashierLineCount, cashierState, cashierBreakCV, CLERKCOUNT)){
    Acquire(printLock);
    PrintString("Manager has woken up a Cashier.\n", sizeof("Manager has woken up a Cashier.\n") );
    Release(printLock);
  }

}

/*Wake up customers in all lines*/
void managerBroacastLine(int* line, int* bribeLine, int lock, int count){
  int i;
  /*DEBUG('s', "DEBUG: MANAGER: BROADCAST acquiring lock %s.\n", lock->getName());*/
  Acquire(lock);
  /*DEBUG('s', "DEBUG: MANAGER: BROADCAST acquired lock %s.\n", lock->getName());*/
  for(i = 0; i < count; i++){
    Broadcast(line[i], lock);
    Broadcast(bribeLine[i], lock);
  }
  Release(lock);
  /*DEBUG('s', "DEBUG: MANAGER: BROADCAST released lock %s.\n", lock->getName());*/
}
void managerBroadcastCustomerLines(){
  /*Wake up all customers in line//So they can go outside*/

  /*App clerks*/
  managerBroacastLine(applicationClerkLineCV, applicationClerkBribeLineCV, applicationClerkLineLock, CLERKCOUNT);
  /*DEBUG('s', "DEBUG: MANAGER: FINISHED BROADCAST to applicaiton lines.\n");*/
  /*Picture clerks*/
  managerBroacastLine(pictureClerkLineCV, pictureClerkBribeLineCV, pictureClerkLineLock, CLERKCOUNT);
  /*DEBUG('s', "DEBUG: MANAGER: FINISHED BROADCAST to picture lines.\n");*/
  /*Passport Clerks*/
  managerBroacastLine(passportClerkLineCV, passportClerkBribeLineCV, passportClerkLineLock, CLERKCOUNT);
  /*DEBUG('s', "DEBUG: MANAGER: FINISHED BROADCAST to passport lines.\n");*/
  /*Cashiers*/
  managerBroacastLine(cashierLineCV, cashierBribeLineCV, cashierLineLock, CLERKCOUNT);
  /*DEBUG('s', "DEBUG: MANAGER: FINISHED BROADCAST to cashier lines.\n");*/
}

/*Checks if a sentor is present...does somehting*/
void managerSenatorCheck(){
  int senatorWaiting;
  int senatorsInside;
  int customersInside;
  int customersOutside;

  
  Acquire(managerLock);

  senatorWaiting = (senatorLineCount > 0);
  senatorsInside = (senatorPresentCount > 0);
  customersInside = (customersPresentCount > 0);
  customersOutside = (passportOfficeOutsideLineCount > 0);

  /*See if a senator is waiting in line...*/
  if(senatorWaiting){
    /*if(!senatorPresentWaitOutSide){ DEBUG('s', "DEBUG: MANAGER NOTICED A SENATOR!.\n"); }*/
    senatorPresentWaitOutSide = 1;

    /*Wake up customers in line so they go outside.*/
    if(customersInside){
      /*DEBUG('s', "DEBUG: MANAGER CUSTOMER PRESENT COUNT: %i.\n", customersPresentCount);*/
      Release(managerLock);
      managerBroadcastCustomerLines();
      /*DEBUG('s', "DEBUG: MANAGER: FINISHED BROADCAST to customers.\n");*/
      return;
    }
  }
  
  if(senatorWaiting && !customersInside){
    /*if(1 || !senatorSafeToEnter) DEBUG('s', "DEBUG: MANAGER: SENATORS SAFE TO ENTER.\n");*/
    senatorSafeToEnter = true;
    senatorLineCV->Broadcast(managerLock);
    /*DEBUG('s', "DEBUG: MANAGER: FINISHED BROADCAST to senators.\n");*/
  }

  if(senatorSafeToEnter && !senatorWaiting && !senatorsInside){
    /*if(senatorSafeToEnter){DEBUG('s', "DEBUG: SENATORS GONE CUSTOMERS COME BACK IN!.\n");}*/
    senatorSafeToEnter = 0;
    senatorPresentWaitOutSide = 0;
    Broadcast(passportOfficeOutsideLineCV, managerLock);
  }


  Release(managerLock);
}/*End managerSenatorCheck*/


void Manager(){
  int i;
  /*Untill End of Simulation*/
  while(1){
    for(i = 0; i < 1000; i++) { 
    

      /*SENATORS*/
      managerSenatorCheck();

      /*Check Lines Wake up Clerk if More than 3 in a line.*/
      managerCheckandWakupClerks();

      /*Check if all the customers are gone and let all the clerks go home*/
      checkEndOfDay();

      Yield(); 
      Yield(); 
      Yield(); 
    }/*Let someone else get the CPU*/
    
    /*Count and print money*/
    /*managerCountMoney();*/
  }

}/*End Manager*/








































int main() {
  int i;

  THEEND = 0;

  senatorPresentWaitOutSide = 0;
  senatorSafeToEnter = 0;

  applicationClerkLineLock = CreateLock();
  pictureClerkLineLock = CreateLock();
  passportClerkLineLock = CreateLock();
  cashierLineLock = CreateLock();
  managerLock = CreateLock();
  printLock = CreateLock();

  applicationClerkBreakCV = CreateCondition();
  pictureClerkBreakCV = CreateCondition();
  passportClerkBreakCV = CreateCondition();
  cashierBreakCV = CreateCondition();
  passportOfficeOutsideLineCV = CreateCondition();
  senatorLineCV = CreateCondition();








  /*Init clerkStates, lineCounts*/
  for(i=0; i<MAXCLERKS; i++){
    applicationClerkState[i] = BUSY;
    pictureClerkState[i] = BUSY;
    passportClerkState[i] = BUSY;
    cashierState[i] = BUSY;

    applicationClerkLineCount[i] = 0;     
    applicationClerkBribeLineCount[i] = 0;    
    pictureClerkLineCount[i] = 0;     
    pictureClerkBribeLineCount[i] = 0;    
    passportClerkLineCount[i] = 0;      
    passportClerkBribeLineCount[i] = 0;   
    cashierLineCount[i] = 0;      
    cashierBribeLineCount[i] = 0;

    applicationClerkSharedData[i] = 0;
    pictureClerkSharedDataSSN[i] = 0;
    pictureClerkSharedDataPicture[i] = 0;
    passportClerkSharedDataSSN[i] = 0;

    applicationClerkLock[i] = CreateLock();
    applicationClerkLineCV[i] = CreateCondition();
    applicationClerkBribeLineCV[i] = CreateCondition();
    applicationClerkCV[i] = CreateCondition();

    pictureClerkLock[i] = CreateLock();
    pictureClerkLineCV[i] = CreateCondition();
    pictureClerkBribeLineCV[i] = CreateCondition();
    pictureClerkCV[i] = CreateCondition();

    passportClerkLock[i] = CreateLock();
    passportClerkLineCV[i] = CreateCondition();
    passportClerkBribeLineCV[i] = CreateCondition();
    passportClerkCV[i] = CreateCondition();

    cashierLock[i] = CreateLock();
    cashierLineCV[i] = CreateCondition();
    cashierBribeLineCV[i] = CreateCondition();
    cashierCV[i] = CreateCondition();
  }

  for(i = 0; i < MAXCUSTOMERS + MAXSENATORS; i++){
    applicationCompletion[i] = 0;
    pictureCompletion[i] = 0;
    passportCompletion[i] = 0;
    passportPunishment[i] = 0;
    cashierSharedDataSSN[i] = 0;
    cashierRejection[i] = 0;
    doneCompletely[i] = 0;
  }


  for(i = 0; i < CUSTOMERCOUNT; i++){
    Fork(Customer);
  }

  /*
  for(i = 0; i < SENATORCOUNT; i++){
    Fork(Senator);
  }*/

/*
  for(i = 0; i < CLERKCOUNT; i++){
    Fork(ApplicationClerk);
    Fork(PictureClerk);
    Fork(PassportClerk);
    Fork(Cashier);
  }*/

  Fork(Manager);

  while(!THEEND){
    Yield();
  }


  Acquire(printLock);
  PrintString("Passport Office Closed.\n", sizeof("Passport Office Closed.\n"));
  Release(printLock);

  Exit(0);
}