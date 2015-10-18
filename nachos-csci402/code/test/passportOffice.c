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
char SENATORTEXT[] = "Senator ";

#define true 1
#define false 0

/***********
* Locks
*********/
int applicationClerkLineLock;
int pictureClerkLineLock;
int passportClerkLineLock;
int cashierLineLock;
int managerLock;

int printLock;  /*For using the PrintSyscalls*/
int SSNLock;
int SSNCount = 0;
int ApplicationMyLineLock;
int ApplicationMyLine;
int PictureMyLineLock;
int PictureMyLine;
int PassportMyLineLock;
int PassportMyLine;
int CashierMyLineLock;
int CashierMyLine;


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
* Probably Broken Utility Functions...
***************************/

/*Used by customerInteractions to return customer/senator text...*/
char* MYTYPE(int VIP){
  if(VIP == 0){
    return CUSTOMERTEXT;
  }else if(VIP == 1){
    return SENATORTEXT;
  }else{
    return "NULL";
  }
}

/* Utility function to pick the shortest line
* Customer has already chosen type of line to get in just needs to pick which line
* Assumptions: The caller has a lock for the given MVs
*Parameters: 
  *lineCount: a vector of the lineCount
  *clerkState: a vector of the clerkState*/
int pickShortestLine(int* pickShortestlineCount, int* pickShortestclerkState){
  int myLine = -1;
  int lineSize = 1000;
  int i;
  for(i=0; i < CLERKCOUNT; i++){
    /*If lineCount < lineSize and clerk is not on break*/
    if(pickShortestlineCount[i] < lineSize && pickShortestclerkState[i] != ONBREAK ){
      myLine = i;
      lineSize = pickShortestlineCount[i];
    }
  }
  return myLine;  /*This is the shortest line*/
}/*End pickShortestLine*/

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

int customerCheckIn(){
  int SSN;
  Acquire(managerLock);
  customersPresentCount++;
  Release(managerLock);

  Acquire(SSNLock);
  SSN = SSNCount;
  SSNCount++;
  Release(SSNLock);
  return SSN;
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







/*The Customer's Interaction with the applicationClerk
*    Get their application accepted by the ApplicationClerk*/
int customerApplicationClerkInteraction(int SSN, int *money, int VIP){
  int myLine = -1;
  char* myType = MYTYPE(VIP);
  int bribe = (*money > 500) && (Rand()%2) && !VIP;/*VIPS dont bribe...*/
  /*I have decided to go to the applicationClerk*/

  /*I should acquire the line lock*/
  Acquire(applicationClerkLineLock);
  /*lock acquired*/

  /*Can I go to counter, or have to wait? Should i bribe?*/
  /*Pick shortest line with clerk not on break*/
  /*Should i get in the regular line else i should bribe?*/
  if(!bribe){ /*Get in regular line*/
    myLine = pickShortestLine(applicationClerkLineCount, applicationClerkState);
  }else{ /*get in bribe line*/
    myLine = pickShortestLine(applicationClerkBribeLineCount, applicationClerkState);
  }
  
  /*I must wait in line*/
  if(applicationClerkState[myLine] != AVAILABLE){
    if(!bribe){
      applicationClerkLineCount[myLine]++;
      /*printf("%s %i has gotten in regular line for ApplicationClerk %i.\n", myType, SSN, myLine);*/

      Acquire(printLock);
      PrintString(myType, 8);
      PrintString(" ", 1);
      PrintInt(SSN);
      PrintString(" has gotten in regular line for ApplicationClerk ", 
          sizeof(" has gotten in regular line for ApplicationClerk ") );
      PrintInt(myLine);
      PrintString(".\n", 2);
      Release(printLock);

      Wait(applicationClerkLineCV[myLine], applicationClerkLineLock);
      applicationClerkLineCount[myLine]--;
      /*See if the clerk for my line signalled me, otherwise check if a senator is here and go outside.*/
      if(applicationClerkState[myLine] != SIGNALEDCUSTOMER){
        Release(applicationClerkLineLock);
        if(customerCheckSenator(SSN))
          return 0;
      }
    }else{
      applicationClerkBribeLineCount[myLine]++;

      /*printf("%s %i has gotten in bribe line for ApplicationClerk %i.\n", myType, SSN, myLine);*/
       Acquire(printLock);
      PrintString(myType, 8);
      PrintString(" ", 1);
      PrintInt(SSN);
      PrintString(" has gotten in bribe line for ApplicationClerk ", 
          sizeof(" has gotten in bribe line for ApplicationClerk ") );
      PrintInt(myLine);
      PrintString(".\n", 2);
      Release(printLock);
      
      Wait(applicationClerkBribeLineCV[myLine], applicationClerkLineLock);
      applicationClerkBribeLineCount[myLine]--;
      /*See if the clerk for my line signalled me, otherwise check if a senator is here and go outside.*/
      if(applicationClerkState[myLine] != SIGNALEDCUSTOMER){
        Release(applicationClerkLineLock);
        if(customerCheckSenator(SSN))
          return 0;
      }
      *money -= 500;
    }
  }
  /*Clerk is AVAILABLE*/
  applicationClerkState[myLine] = BUSY;
  Release(applicationClerkLineLock);
  /*Lets talk to clerk*/
  Acquire(applicationClerkLock[myLine]);
  /*Give my data to my clerk*/
  /*We already have a lock so put my SSN in applicationClerkSharedData*/
  applicationClerkSharedData[myLine] = SSN;
  /*printf("%s %i has given SSN %i to ApplicationClerk %i.\n", myType, SSN, SSN, myLine);*/
  Acquire(printLock);
      PrintString(myType, 8);
      PrintString(" ", 1);
      PrintInt(SSN);
      PrintString(" has given SSN ", sizeof(" has given SSN "));
      PrintInt(SSN);
      PrintString(" to ApplicationClerk ", sizeof(" to ApplicationClerk "));
      PrintInt(myLine);
      PrintString(".\n", 2);
  Release(printLock);
  Signal(applicationClerkCV[myLine], applicationClerkLock[myLine]);
  /*Wait for clerk to do their job*/
  Wait(applicationClerkCV[myLine], applicationClerkLock[myLine]);
  
  /*Done*/
  Release(applicationClerkLock[myLine]);
  return 1;
}/*End customerApplicationClerkInteraction*/






/*The Customer's Interaction with the pictureClerk
//Get their picture accepted by the pictureClerk*/
int customerPictureClerkInteraction(int SSN, int *money, int VIP){
  int myLine = -1;
  char* myType = MYTYPE(VIP);
  int bribe = (*money > 500) && (Rand()%2) && !VIP;

  Acquire(pictureClerkLineLock);

  if(!bribe){
    myLine = pickShortestLine(pictureClerkLineCount, pictureClerkState);
  }else{
    myLine = pickShortestLine(pictureClerkBribeLineCount, pictureClerkState);
  }
  
  if(pictureClerkState[myLine] != AVAILABLE){
    if(!bribe){
      pictureClerkLineCount[myLine]++;
      /*printf("%s %i has gotten in regular line for PictureClerk %i.\n", myType, SSN, myLine);*/
       Acquire(printLock);
          PrintString(myType, 8);
          PrintString(" ", 1);
          PrintInt(SSN);
          PrintString(" has gotten in regular line for PictureClerk ", 
              sizeof(" has gotten in regular line for PictureClerk ") );
          PrintInt(myLine);
          PrintString(".\n", 2);
      Release(printLock);
      Wait(pictureClerkLineCV[myLine], pictureClerkLineLock);
      pictureClerkLineCount[myLine]--;
      if(pictureClerkState[myLine] != SIGNALEDCUSTOMER){
        Release(pictureClerkLineLock);
        if(customerCheckSenator(SSN))
          return false;
      }
    }else{
      pictureClerkBribeLineCount[myLine]++;
      /*printf("%s %i has gotten in bribe line for PictureClerk %i.\n", myType, SSN, myLine);*/
      Acquire(printLock);
          PrintString(myType, 8);
          PrintString(" ", 1);
          PrintInt(SSN);
          PrintString(" has gotten in bribe line for PictureClerk ", 
              sizeof(" has gotten in bribe line for PictureClerk ") );
          PrintInt(myLine);
          PrintString(".\n", 2);
      Release(printLock);

      Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock);
      pictureClerkBribeLineCount[myLine]--;
      if(pictureClerkState[myLine] != SIGNALEDCUSTOMER){
        Release(pictureClerkLineLock);
        if(customerCheckSenator(SSN))
          return false;
      }
      *money -= 500;
    }
  }

  pictureClerkState[myLine] = BUSY;
  Release(pictureClerkLineLock);

  Acquire(pictureClerkLock[myLine]);
 
  pictureClerkSharedDataSSN[myLine] = SSN;
  Acquire(printLock);
      PrintString(myType, 8);
      PrintString(" ", 1);
      PrintInt(SSN);
      PrintString(" has given SSN ", sizeof(" has given SSN "));
      PrintInt(SSN);
      PrintString(" to PictureClerk ", sizeof(" to PictureClerk ") );
      PrintInt(myLine);
      PrintString(".\n", 2);
  Release(printLock);


  Signal(pictureClerkCV[myLine], pictureClerkLock[myLine]);
  Wait(pictureClerkCV[myLine], pictureClerkLock[myLine]);


  while(pictureClerkSharedDataPicture[myLine] == 0) {
    if(Rand()%10 > 7) {
      Acquire(printLock);
          PrintString(myType, 8);
          PrintString(" ", 1);
          PrintInt(SSN);
          PrintString(" does not like their picture from PictureClerk ",
               sizeof(" does not like their picture from PictureClerk "));
          PrintInt(myLine);
          PrintString(".\n", 2);
      Release(printLock);
      pictureClerkSharedDataPicture[myLine] = 0;
      Signal(pictureClerkCV[myLine], pictureClerkLock[myLine]);

      Wait(pictureClerkCV[myLine], pictureClerkLock[myLine]);
    }
    else {
      /*printf("%s %i does like their picture from PictureClerk %i.\n", myType, SSN, myLine);*/
      Acquire(printLock);
          PrintString(myType, 8);
          PrintString(" ", 1);
          PrintInt(SSN);
          PrintString(" does like their picture from PictureClerk ",
               sizeof(" does like their picture from PictureClerk "));
          PrintInt(myLine);
          PrintString(".\n", 2);
      Release(printLock);
      pictureClerkSharedDataPicture[myLine] = 1;
      Signal(pictureClerkCV[myLine], pictureClerkLock[myLine]);

      Wait(pictureClerkCV[myLine], pictureClerkLock[myLine]);
    }
  }
  Release(pictureClerkLock[myLine]);

  return true;
}/*End customerPictureClerkInteraction*/




int customerPassportClerkInteraction(int SSN, int *money, int VIP){
  int myLine = -1;
  char* myType = MYTYPE(VIP);
  int bribe = (money > 500) && (Rand()%2) && !VIP;

  Acquire(passportClerkLineLock);


  if(!bribe){
    myLine = pickShortestLine(passportClerkLineCount, passportClerkState);
  }else{ 
    myLine = pickShortestLine(passportClerkBribeLineCount, passportClerkState);
  }

  if(passportClerkState[myLine] != AVAILABLE){
    if(!bribe){
      passportClerkLineCount[myLine]++;
      Acquire(printLock);
          PrintString(myType, 8);
          PrintString(" ", 1);
          PrintInt(SSN);
          PrintString(" has gotten in regular line for PassportClerk ", 
              sizeof(" has gotten in regular line for PassportClerk ") );
          PrintInt(myLine);
          PrintString(".\n", 2);
      Release(printLock);
      Wait(passportClerkLineCV[myLine], passportClerkLineLock);
      passportClerkLineCount[myLine]--;
      if(passportClerkState[myLine] != SIGNALEDCUSTOMER){
        Release(passportClerkLineLock);
        if(customerCheckSenator(SSN))
          return false;
      }
    }else{
      passportClerkBribeLineCount[myLine]++;
      Acquire(printLock);
          PrintString(myType, 8);
          PrintString(" ", 1);
          PrintInt(SSN);
          PrintString(" has gotten in bribe line for PassportClerk ", 
              sizeof(" has gotten in bribe line for PassportClerk ") );
          PrintInt(myLine);
          PrintString(".\n", 2);
      Release(printLock);
      Wait(passportClerkBribeLineCV[myLine], passportClerkLineLock);
      passportClerkBribeLineCount[myLine]--;
      if(passportClerkState[myLine] != SIGNALEDCUSTOMER){
        Release(passportClerkLineLock);
        if(customerCheckSenator(SSN))
          return false;
      }
      money -= 500;
    }
  }


  passportClerkState[myLine] = BUSY;
  Release(passportClerkLineLock);

  Acquire(passportClerkLock[myLine]);


  passportClerkSharedDataSSN[myLine] = SSN;
  Acquire(printLock);
      PrintString(myType, 8);
      PrintString(" ", 1);
      PrintInt(SSN);
      PrintString(" has given SSN ", 
          sizeof(" has given SSN ") );
      PrintInt(SSN);
      PrintString(" to PassportClerk ", sizeof(" to PassportClerk "));
      PrintInt(myLine);
      PrintString(".\n", 2);
  Release(printLock);
  Signal(passportClerkCV[myLine], passportClerkLock[myLine]);


  Wait(passportClerkCV[myLine], passportClerkLock[myLine]);
  if(passportPunishment[SSN] == 1) {
    Acquire(printLock);
      PrintString(myType, 8);
      PrintString(" ", 1);
      PrintInt(SSN);
      PrintString(" has gone to PassportClerk ", 
          sizeof(" has gone to PassportClerk ") );
      PrintInt(myLine);
      PrintString(" to PassportClerk ", sizeof(" to PassportClerk "));
      PrintInt(myLine);
      PrintString(" too soon. They are going to the back of the line.\n", 
           sizeof(" too soon. They are going to the back of the line.\n"));
  Release(printLock);
    Release(passportClerkLock[myLine]);
    return false;
  }

  Release(passportClerkLock[myLine]);

  return true;

}/*//End of customerPassportClerkInteraction*/


























/**********************
* Customer
**********************/
void Customer(){
  int appClerkDone = 0;
  int pictureClerkDone = 0;
  int passportClerkDone = 0;
  int cashierDone = 0;
  int SSN = -1;
  int money = (Rand()%4)*500 + 100;
  int appClerkFirst = Rand() % 2;

  SSN = customerCheckIn();


  while(1){

    /*Check if a senator is present and wait outside if there is.*/
    customerCheckSenator(SSN);

    if( !(appClerkDone) && (appClerkFirst || pictureClerkDone) ){ /*Go to applicationClerk*/
      appClerkDone = customerApplicationClerkInteraction(SSN, &money, 0);
    }
    else if( !pictureClerkDone ){
      /*Go to the picture clerk*/
      pictureClerkDone = customerPictureClerkInteraction(SSN, &money, 0);
    }/*
    else if(!passportClerkDone){
      passportClerkDone = customerPassportClerkInteraction(SSN, money);
      if (!passportClerkDone) { for (int i = 0; i < rand() % 901 + 100; i++) { currentThread->Yield(); } }
    }
    else if(!cashierDone){
      cashierDone = customerCashierInteraction(SSN, money);
      if (!cashierDone) { for (int i = 0; i < rand() % 901 + 100; i++) { currentThread->Yield(); } }
    }*/
    else{
      /*This terminates the customer should go at end.*/
      Acquire(printLock);
      PrintString("Customer ", sizeof("Customer "));
      PrintInt(SSN);
      PrintString(" money: ", sizeof(" money: "));
      PrintInt(money);
      PrintString("\n", 1);
      Release(printLock);
      customerCheckOut(SSN);
    }
  }

  Exit(0);
}/*End Customer*/

























/****************************************************************************
*****************************************************************************
*****************************************************************************
*****************************************************************************
*
* Clerks
*
*****************************************************************************
****************************************************************************/


/*This may be necessary to check for race conditions while a senator is waiting outside
 * Before the customer leaves their line the clerk might think they are able to call them*/
int clerkCheckForSenator(){
  int i;
  /*DEBUG('s', "DEBUG: Clerk bout to check for senator.\n");*/
  Acquire(managerLock);
  if(senatorPresentWaitOutSide && !senatorSafeToEnter){
    Release(managerLock);
    /*Lets just wait a bit...
    printf("DEBUG: Clerk yielding for senator.\n");*/
    for(i = 0; i < Rand()%780 + 20; i++) { Yield(); }
    return 1;
  }
  Release(managerLock);
  return 0;
}








/**********************************
* ApplicationClerk
**********************************/

/*Utility for applicationClerk to gon on brak
* Assumptions: called with clerkLineLock*/
void applicationClerkcheckAndGoOnBreak(int myLine){
  /*Only go on break if there is at least one other clerk*/
  int freeOrAvailable = 0;
  int i;
  for(i = 0; i < CLERKCOUNT; i++){
    if(i != myLine && ( applicationClerkState[i] == AVAILABLE || applicationClerkState[i] == BUSY ) ){
      freeOrAvailable = 1;
      break;
    }
  }
  /*There is at least one clerk...go on a break.*/
  if(freeOrAvailable){
    applicationClerkState[myLine] = ONBREAK;

    Acquire(printLock);
    PrintString("ApplicationClerk ", sizeof("ApplicationClerk ") );
    PrintInt(myLine);
    PrintString(" is going on break.\n", sizeof(" is going on break.\n") );
    Release(printLock);

    Wait(applicationClerkBreakCV, applicationClerkLineLock);
    applicationClerkState[myLine] = BUSY;

    Acquire(printLock);
    PrintString("ApplicationClerk ", sizeof("ApplicationClerk ") );
    PrintInt(myLine);
    PrintString(" is coming off break.\n", sizeof(" is coming off break.\n") );
    Release(printLock);

  }else{
    /*If everyone is on break...
    * applicationClerkState[myLine] = AVAILABLE;*/
    Release(applicationClerkLineLock);
    /*Should we go to sleep?*/
    Acquire(managerLock);
    if(checkedOutCount == (CUSTOMERCOUNT + SENATORCOUNT)){Release(managerLock); Exit(0);}
    Release(managerLock);
    Yield();
    Acquire(applicationClerkLineLock);
  }
  /*applicationClerkState[myLine] = AVAILABLE;*/
}

int getMyApplicationLine(){
  int myLine;
  Acquire(ApplicationMyLineLock);
  myLine = ApplicationMyLine++;
  Release(ApplicationMyLineLock);
  return myLine;
}

/*ApplicationClerk - an application clerk accepts a completed Application. 
// A completed Application requires an "completed" application and a Customer "social security number".
// You can assume that the Customer enters the passport office with a completed passport application. 
// The "social security number" can be a random number, or a sequentially increasing number. 
// In any event, it must be a unique number for each Customer.
//
// PassportClerks "record" that a Customer has a completed application. 
// The Customer must pass the application to the PassportClerk. 
// This consists of giving the ApplicationClerk their Customer "social security number" - their personal number.
// The application is assumed to be passed, it is not explicitly provided in the shared data between the 2 threads.
// The ApplicationClerk then "records" that a Customer, with the provided social security number, has a completed application. 
  //This information is used by the PassportClerk. Customers are told when their application has been "filed".
// Any money received from Customers wanting to move up in line must be added to the ApplicationClerk received money amount.
// ApplicationClerks go on break if they have no Customers in their line.
// There is always a delay in an accepted application being "filed".
// This is determined by a random number of 'currentThread->Yield() calls - the number is to vary from 20 to 100.*/
void ApplicationClerk(){
  int myLine;
  int money = 0;
  int customerFromLine;/*0 no line, 1 bribe line, 2 regular line*/
  int customerSSN;
  int i;

  myLine = getMyApplicationLine();


  while(1){

    if(clerkCheckForSenator()) continue; /*Waiting for senators to enter just continue.*/

    Acquire(applicationClerkLineLock);

    /*If there is someone in my bribe line*/
    if(applicationClerkBribeLineCount[myLine] > 0){
      customerFromLine = 1;
      Signal(applicationClerkBribeLineCV[myLine], applicationClerkLineLock);
      applicationClerkState[myLine] = SIGNALEDCUSTOMER;
    }else if(applicationClerkLineCount[myLine] > 0){/*if there is someone in my regular line*/
      customerFromLine = 2;
      Signal(applicationClerkLineCV[myLine], applicationClerkLineLock);
      applicationClerkState[myLine] = SIGNALEDCUSTOMER;
    }else{
      /*No Customers
      //Go on break if there is another clerk*/
      customerFromLine = 0;
      applicationClerkcheckAndGoOnBreak(myLine);
      Release(applicationClerkLineLock);
    }

    /*Should only do this when we have a customer...*/
    if(customerFromLine != 0){

      /*printf("ApplicationClerk %i has signalled a Customer to come to their counter.\n", myLine);*/
      Acquire(printLock);
      PrintString("ApplicationClerk ", sizeof("ApplicationClerk "));
      PrintInt(myLine);
      PrintString(" has signalled a Customer to come to their counter.\n",
         sizeof(" has signalled a Customer to come to their counter.\n"));
      Release(printLock);

      Acquire(applicationClerkLock[myLine]);
      Release(applicationClerkLineLock);
      /*wait for customer data*/
      Wait(applicationClerkCV[myLine], applicationClerkLock[myLine]);
      /*Customer Has given me their SSN?
      //And I have a lock*/
      customerSSN = applicationClerkSharedData[myLine];
      
      /*Customer from bribe line? //maybe should be separate signalwait  ehh?*/
      if(customerFromLine == 1){
        money += 500;
        /*printf("ApplicationClerk %i has received $500 from Customer %i.\n", myLine, customerSSN);*/
       Acquire(printLock);
          PrintString("ApplicationClerk ", sizeof("ApplicationClerk "));
          PrintInt(myLine);
          PrintString(" has received $500 from Customer ", sizeof(" has received $500 from Customer "));
          PrintInt(customerSSN);
          PrintString(".\n", 2);
        Release(printLock);
        Yield();/*Just to change things up a bit.*/
      }
      

      /*printf("ApplicationClerk %i has received SSN %i from Customer %i.\n", myLine, customerSSN, customerSSN);*/
      Acquire(printLock);
          PrintString("ApplicationClerk ", sizeof("ApplicationClerk "));
          PrintInt(myLine);
          PrintString(" has received SSN ", sizeof(" has received SSN "));
          PrintInt(customerSSN);
          PrintString(" from Customer ", sizeof(" from Customer "));
          PrintInt(customerSSN);
          PrintString(".\n", 2);
        Release(printLock);
      
      /*Signal Customer that I'm Done.*/
      Signal(applicationClerkCV[myLine], applicationClerkLock[myLine]);
      Release(applicationClerkLock[myLine]);

      /*yield for filing time*/
      for(i = 0; i < Rand()%81 + 20; i++) { Yield(); }
      
      /*TODO: NEED TO ACQUIRE A LOCK FOR THIS!!*/
      applicationCompletion[customerSSN] = 1;
      /*printf("ApplicationClerk %i has recorded a completed application for Customer %i.\n", myLine, customerSSN);*/
      Acquire(printLock);
          PrintString("ApplicationClerk ", sizeof("ApplicationClerk "));
          PrintInt(myLine);
          PrintString(" has recorded a completed application for Customer ", 
               sizeof(" has recorded a completed application for Customer "));
          PrintInt(customerSSN);
          PrintString(".\n", 2);
        Release(printLock);
    }/*end if have customer*/

  }

}/*End ApplicationClerk*/


















/*****************************************
* PictureClerk
*******************************************/


/*Utility for applicationClerk to gon on brak
// Assumptions: called with clerkLineLock*/
void pictureClerkcheckAndGoOnBreak(int myLine){
  int i;

  int freeOrAvailable = false;
  for(i = 0; i < CLERKCOUNT; i++){
    if(i != myLine && ( pictureClerkState[i] == AVAILABLE || pictureClerkState[i] == BUSY ) ){
      freeOrAvailable = true;
      break;
    }
  }

  if(freeOrAvailable){
    pictureClerkState[myLine] = ONBREAK;
    Acquire(printLock);
      PrintString("PictureClerk ", sizeof("PictureClerk ") );
      PrintInt(myLine);
      PrintString(" is going on break.\n", sizeof(" is going on break.\n") );
    Release(printLock);
    Wait(pictureClerkBreakCV, pictureClerkLineLock);
    pictureClerkState[myLine] = BUSY;
    Acquire(printLock);
      PrintString("PictureClerk ", sizeof("PictureClerk ") );
      PrintInt(myLine);
      PrintString(" is coming off break.\n", sizeof(" is coming off break.\n") );
    Release(printLock);
  }else{

    Release(pictureClerkLineLock);

    Acquire(managerLock);
    if(checkedOutCount == (CUSTOMERCOUNT + SENATORCOUNT)){Release(managerLock); Exit(0);}
    Release(managerLock);
    Yield();

    Acquire(pictureClerkLineLock);
  }
  /*applicationClerkState[myLine] = AVAILABLE;*/
}

int PictureGetMyLine(){
  int myLine;
  Acquire(PictureMyLineLock);
  myLine = PictureMyLine++;
  Release(PictureMyLineLock);
  return myLine;
}

void PictureClerk(){
    int myLine;
    int money = 0;
    int customerFromLine;/*0 no line, 1 bribe line, 2 regular line*/
    int i;
    int customerSSN;

    myLine = PictureGetMyLine();

    while(1){
  
      if(clerkCheckForSenator()) continue; /*Waiting for senators to enter just continue.*/

      Acquire(pictureClerkLineLock);
      /*DEBUG('p', "DEBUG: PictureClerk %i pICTURECLERKLINELOCK acquired top of while\n", myLine);*/

      /*If there is someone in my bribe line*/
      if(pictureClerkBribeLineCount[myLine] > 0){
        customerFromLine = 1;
        Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock);
        pictureClerkState[myLine] = SIGNALEDCUSTOMER;
      }else if(pictureClerkLineCount[myLine] > 0){/*if there is someone in my regular line*/
        customerFromLine = 2;
        Signal(pictureClerkLineCV[myLine], pictureClerkLineLock);
        pictureClerkState[myLine] = SIGNALEDCUSTOMER;
      }else{
        /*Go on a break!*/
        customerFromLine = 0;
        pictureClerkcheckAndGoOnBreak(myLine);
        Release(pictureClerkLineLock);
        /*DEBUG('p', "DEBUG: PictureClerk %i pICTURECLERKLINELOCK released after checkbreak.\n", myLine);*/
      }

      /*Should only do this when we are BUSY? We have a customer...*/
      if(customerFromLine != 0){
        Acquire(printLock);
          PrintString("PictureClerk ", sizeof("PictureClerk "));
          PrintInt(myLine);
          PrintString(" has signalled a Customer to come to their counter.\n" ,
               sizeof(" has signalled a Customer to come to their counter.\n"));
        Release(printLock);

        Acquire(pictureClerkLock[myLine]);
      /*  pictureClerkSharedDataPicture[myLine] = 0;*/
        Release(pictureClerkLineLock);
        /*DEBUG('p', "DEBUG: PictureClerk %i pICTURECLERKLINELOCK released after signalling customer.\n", myLine);*/
        /*wait for customer data*/
        Wait(pictureClerkCV[myLine], pictureClerkLock[myLine]);
        /*Customer Has given me their SSN?
        //And I have a lock*/
        customerSSN = pictureClerkSharedDataSSN[myLine];
        /*Customer from bribe line? //maybe should be separate signalwait  ehh?*/
      if(customerFromLine == 1){
        money += 500;
        /*printf("PictureClerk %i has received $500 from Customer %i.\n", myLine, customerSSN);*/
        Acquire(printLock);
          PrintString("PictureClerk ", sizeof("PictureClerk "));
          PrintInt(myLine);
          PrintString(" has received $500 from Customer " ,
               sizeof(" has received $500 from Customer "));
          PrintInt(customerSSN);
          PrintString(".\n", 2);
        Release(printLock);

        Yield();/*Just to change things up a bit.*/
      }
      
        /*printf("PictureClerk %i has received SSN %i from Customer %i.\n", myLine, customerSSN, customerSSN);*/
        Acquire(printLock);
          PrintString("PictureClerk ", sizeof("PictureClerk "));
          PrintInt(myLine);
          PrintString("  has received SSN ", sizeof("  has received SSN ") );
          PrintInt(customerSSN);
          PrintString(" from Customer " , sizeof(" from Customer "));
          PrintInt(customerSSN);
          PrintString(".\n", 2);
        Release(printLock);

        pictureClerkSharedDataPicture[myLine] = 0;
        while(pictureClerkSharedDataPicture[myLine] == 0) {
          /*Taking picture*/
          /*printf("PictureClerk %i has taken a picture of Customer %i.\n", myLine, customerSSN);*/
          Acquire(printLock);
              PrintString("PictureClerk ", sizeof("PictureClerk "));
              PrintInt(myLine);
              PrintString(" has taken a picture of Customer ", sizeof(" has taken a picture of Customer ") );
              PrintInt(customerSSN);
              PrintString(".\n", 2);
          Release(printLock);


          /*Signal Customer that I'm Done and show them the picture. Then wait for response.*/
          Signal(pictureClerkCV[myLine], pictureClerkLock[myLine]);
          Wait(pictureClerkCV[myLine], pictureClerkLock[myLine]);
          if(pictureClerkSharedDataPicture[myLine] == 0)  {
            /*printf("PictureClerk %i has has been told that Customer %i does not like their picture.\n", myLine, customerSSN);*/
            Acquire(printLock);
              PrintString("PictureClerk ", sizeof("PictureClerk "));
              PrintInt(myLine);
              PrintString(" has taken been told that Customer ", sizeof(" has taken been told that Customer ") );
              PrintInt(customerSSN);
              PrintString(" does not like their picture.\n", sizeof(" does not like their picture.\n"));
            Release(printLock);
          }

        }
        /*printf("PictureClerk %i has has been told that Customer %i does like their picture.\n", myLine, customerSSN);*/
        Acquire(printLock);
          PrintString("PictureClerk ", sizeof("PictureClerk "));
          PrintInt(myLine);
          PrintString(" has taken been told that Customer ", sizeof(" has taken been told that Customer ") );
          PrintInt(customerSSN);
          PrintString(" does like their picture.\n", sizeof(" does like their picture.\n"));
        Release(printLock);
        /*Yield before submitting.*/
        /*Signal Customer that I'm Done.*/
        Signal(pictureClerkCV[myLine], pictureClerkLock[myLine]);
        for(i = 0; i < Rand()%81 + 20; i++) { Yield(); }
        /*printf("PictureClerk %i has recorded a completed picture for Customer %i.\n", myLine, customerSSN);*/
          Acquire(printLock);
              PrintString("PictureClerk ", sizeof("PictureClerk "));
              PrintInt(myLine);
              PrintString(" has recorded a completed picture for Customer ", sizeof(" has recorded a completed picture for Customer ") );
              PrintInt(customerSSN);
              PrintString(".\n", sizeof(".\n"));
            Release(printLock);
        pictureCompletion[customerSSN] = 1;


        Release(pictureClerkLock[myLine]);
      }
  
    }

}/*End PictureClerk*/









void passportClerkcheckAndGoOnBreak(int myLine){

  int freeOrAvailable = false;
  int i;
  for(i = 0; i < CLERKCOUNT; i++){
    if(i != myLine && ( passportClerkState[i] == AVAILABLE || passportClerkState[i] == BUSY ) ){
      freeOrAvailable = true;
      break;
    }
  }

  if(freeOrAvailable){
    passportClerkState[myLine] = ONBREAK;
    Acquire(printLock);
      PrintString("PassportClerk ", sizeof("PassportClerk ") );
      PrintInt(myLine);
      PrintString(" is going on break.\n", sizeof(" is going on break.\n") );
    Release(printLock);
    Wait(passportClerkBreakCV, passportClerkLineLock);
    passportClerkState[myLine] = BUSY;
    Acquire(printLock);
      PrintString("PassportClerk ", sizeof("PassportClerk ") );
      PrintInt(myLine);
      PrintString(" is coming off break.\n", sizeof(" is coming off break.\n") );
    Release(printLock);
  }else{

    Release(passportClerkLineLock);

    Acquire(managerLock);
    if(checkedOutCount == (CUSTOMERCOUNT + SENATORCOUNT)){Release(managerLock); Exit(0);}
    Release(managerLock);
    Yield();
    Acquire(passportClerkLineLock);
  }
  /*passportClerkState[myLine] = AVAILABLE;*/
}

int PassportGetMyLine(){
  int myLine;
  Acquire(PassportMyLineLock);
  myLine = PassportMyLine++;
  Release(PassportMyLineLock);
  return myLine;
}


void PassportClerk(){
  int myLine;
  int money = 0;
  int customerFromLine;/*0 no line, 1 bribe line, 2 regular line*/
  int i;
  int customerSSN;
  
  myLine = PassportGetMyLine();

  while(true){

    if(clerkCheckForSenator()) continue; 

    Acquire(passportClerkLineLock);

    if(passportClerkBribeLineCount[myLine] > 0){
      customerFromLine = 1;
      Signal(passportClerkBribeLineCV[myLine], passportClerkLineLock);
      passportClerkState[myLine] = SIGNALEDCUSTOMER;
    }else if(passportClerkLineCount[myLine] > 0){
      customerFromLine = 2;
      Signal(passportClerkLineCV[myLine], passportClerkLineLock);
      passportClerkState[myLine] = SIGNALEDCUSTOMER;
    }else{
      customerFromLine = 0;
      passportClerkcheckAndGoOnBreak(myLine);
      Release(passportClerkLineLock);
    }


    if(customerFromLine != 0){
      Acquire(printLock);
        PrintString("PassportClerk ", sizeof("PassportClerk ") );
        PrintInt(myLine);
        PrintString(" has signalled a Customer to come to their counter.\n",
             sizeof(" has signalled a Customer to come to their counter.\n") );
      Release(printLock);
      Acquire(passportClerkLock[myLine]);
      Release(passportClerkLineLock);

      Wait(passportClerkCV[myLine], passportClerkLock[myLine]);


      customerSSN = passportClerkSharedDataSSN[myLine];

      if(customerFromLine == 1){
        money += 500;
        /*printf("PassportClerk %i has received $500 from Customer %i.\n", myLine, customerSSN);*/
        Acquire(printLock);
            PrintString("PassportClerk ", sizeof("PassportClerk ") );
            PrintInt(myLine);
            PrintString("has received $500 from Customer ",
                 sizeof("has received $500 from Customer ") );
            PrintInt(customerSSN);
            PrintString(".\n", 2);
        Release(printLock);
        Yield();
      }
      
       Acquire(printLock);
            PrintString("PassportClerk ", sizeof("PassportClerk ") );
            PrintInt(myLine);
            PrintString(" has received SSN ", sizeof(" has received SSN ") );
            PrintInt(customerSSN);
            PrintString(" from Customer ", sizeof(" from Customer ") );
            PrintInt(customerSSN);
            PrintString(".\n", 2);
        Release(printLock);
      
      //Do my job - customer waiting
      if(!(applicationCompletion[customerSSN] == 1 && pictureCompletion[customerSSN] == 1)) {
        passportPunishment[customerSSN] = 1;
        Acquire(printLock);
            PrintString("PassportClerk ", sizeof("PassportClerk ") );
            PrintInt(myLine);
            PrintString(" has determined that Customer ", sizeof(" has determined that Customer ") );
            PrintInt(customerSSN);
            PrintString(" does not have both their application and picture completed.\n",
                 sizeof(" does not have both their application and picture completed.\n") );
        Release(printLock);


        Signal(passportClerkCV[myLine], passportClerkLock[myLine]);
      }
      else {
        passportPunishment[customerSSN] = 0;
       
        Acquire(printLock);
            PrintString("PassportClerk ", sizeof("PassportClerk ") );
            PrintInt(myLine);
            PrintString(" has determined that Customer ", sizeof(" has determined that Customer ") );
            PrintInt(customerSSN);
            PrintString(" has both their application and picture completed.\n",
                 sizeof(" has both their application and picture completed.\n") );
        Release(printLock);

        passportCompletion[customerSSN] = true;

        Signal(passportClerkCV[myLine], passportClerkLock[myLine]);
        for(i = 0; i < Rand()%81 + 20; i++) {Yield(); }
        printf("PassportClerk %i has recorded Customer %i passport documentation.\n", myLine, customerSSN);
        Acquire(printLock);
            PrintString("PassportClerk ", sizeof("PassportClerk ") );
            PrintInt(myLine);
            PrintString(" has recorded Customer ", sizeof(" has recorded Customer ") );
            PrintInt(customerSSN);
            PrintString(" passport documentation.\n",
                 sizeof(" passport documentation.\n") );
        Release(printLock);
      }

      Release(passportClerkLock[myLine]);
    }

  }
  



}/*End PassportClerk*/
























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
    senatorSafeToEnter = 1;
    Broadcast(senatorLineCV, managerLock);
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
  SSNLock = CreateLock();
  SSNCount = 0;
  ApplicationMyLineLock = CreateLock();
  ApplicationMyLine = 0;
  PictureMyLineLock = CreateLock();
  PictureMyLine = 0;
  PassportMyLineLock = CreateLock();
  PassportMyLine = 0;
  CashierMyLineLock = CreateLock();
  CashierMyLine = 0;

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


  for(i = 0; i < CLERKCOUNT; i++){
    Fork(ApplicationClerk);
    Fork(PictureClerk);
    Fork(PassportClerk);/*
    Fork(Cashier);*/
  }

  Fork(Manager);

  while(!THEEND){
    Yield();
  }


  Acquire(printLock);
  PrintString("Passport Office Closed.\n", sizeof("Passport Office Closed.\n"));
  Release(printLock);

  Exit(0);
}