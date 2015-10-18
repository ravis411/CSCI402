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
  PrintString("Customer ", sizeof("Customer ")); 
  PrintInt(SSN);
  PrintString(" is going outside the PassportOffice because there is a Senator present.\n", 
    sizeof(" is going outside the PassportOffice because there is a Senator present.\n"));

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
  PrintString("Customer ", sizeof("Customer ") );
  PrintInt(SSN);
  PrintString(" is leaving the Passport Office.\n", sizeof(" is leaving the Passport Office.\n"));
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
    //All the customers are gone
    //Lets all go to sleep*/
    THEEND = 1;
    Release(managerLock);

  /*currentThread->Finish();*/
    Exit(0);
  }
  Release(managerLock);
}


void Manager(){
  int i;
  //Untill End of Simulation
  while(1){
    for(i = 0; i < 1000; i++) { 
    

      /*SENATORS*/
      /*managerSenatorCheck();*/

      /*Check Lines Wake up Clerk if More than 3 in a line.*/
      /*managerCheckandWakupClerks();*/

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



  PrintString("Passport Office Closed.\n", sizeof("Passport Office Closed.\n"));


  Exit(0);
}