
// Group 25
// Part2.cc

#ifndef PROJ1_PART2_CC
#define PROJ1_PART2_CC

#include "synch.h"
#include "list.h"
#include "vector"
#include <stdlib.h> 

//Settings Variables 
//TODO:These should be more dynamic

int CLERKCOUNT = 2;		//The number of clerks
int CUSTOMERCOUNT = 3; 	//Number of customers


//Globals or constants
enum CLERKSTATE {AVAILABLE, BUSY, ONBREAK};				//enum for the CLERKSTATE

int checkedOutCount = 0;	//For the manager

///////////////////////////////////
//Initialize Locks, CVS, and Monitors?
//Locks
Lock *applicationClerkLineLock = new Lock("applicationClerkLineLock");	//The applicationClerkLineLock
Lock *pictureClerkLineLock = new Lock("pictureClerkLineLock");	//The applicationClerkLineLock
Lock *passportClerkLineLock = new Lock("passportClerkLineLock");
Lock *cashierLineLock = new Lock("cashierLineLock");
Lock *managerLock = new Lock("managerLock");	//Lock for the manager
std::vector<Lock*> applicationClerkLock;
std::vector<Lock*> pictureClerkLock;
std::vector<Lock*> passportClerkLock;
std::vector<Lock*> cashierLock;
//CVS
std::vector<Condition*> applicationClerkLineCV;
std::vector<Condition*> applicationClerkBribeLineCV;	//applicationClerk CVs
std::vector<Condition*> applicationClerkCV;
Condition *applicationClerkBreakCV = new Condition("applicationClerkBreakCV");//To keep track of clerks on break

std::vector<Condition*> pictureClerkLineCV;
std::vector<Condition*> pictureClerkBribeLineCV;	//pictureClerk CVs
std::vector<Condition*> pictureClerkCV;
Condition *pictureClerkBreakCV = new Condition("pictureClerkBreakCV");//To keep track of clerks on break

std::vector<Condition*> passportClerkLineCV;
std::vector<Condition*> passportClerkBribeLineCV;	//passportClerk CVs
std::vector<Condition*> passportClerkCV;
Condition *passportClerkBreakCV = new Condition("passportClerkBreakCV");//To keep track of clerks on break

std::vector<Condition*> cashierLineCV;
std::vector<Condition*> cashierBribeLineCV;	//passportClerk CVs
std::vector<Condition*> cashierCV;
Condition *cashierBreakCV = new Condition("cashierBreakCV");//To keep track of clerks on break


//States
std::vector<CLERKSTATE> applicationClerkState(CLERKCOUNT, BUSY);	//applicationClerkState
std::vector<CLERKSTATE> pictureClerkState(CLERKCOUNT, BUSY);	//applicationClerkState
std::vector<CLERKSTATE> passportClerkState(CLERKCOUNT, BUSY);	//applicationClerkState
std::vector<CLERKSTATE> cashierState(CLERKCOUNT, BUSY);	//applicationClerkState
//LineCounts
std::vector<int> applicationClerkLineCount(CLERKCOUNT, 0);			//applicationClerkLineCount
std::vector<int> applicationClerkBribeLineCount(CLERKCOUNT, 0);		//applicationClerkBribeLineCount
std::vector<int> pictureClerkLineCount(CLERKCOUNT, 0);			//pictureClerkLineCount
std::vector<int> pictureClerkBribeLineCount(CLERKCOUNT, 0);		//pictureClerkBribeLineCount
std::vector<int> passportClerkLineCount(CLERKCOUNT, 0);			//passportClerkLineCount
std::vector<int> passportClerkBribeLineCount(CLERKCOUNT, 0);		//passportClerkBribeLineCount
std::vector<int> cashierLineCount(CLERKCOUNT, 0);			//cashierLineCount
std::vector<int> cashierBribeLineCount(CLERKCOUNT, 0);		//cashierBribeLineCount
//Shared Data //Should only be accessed with the corresponding lock / cv
std::vector<int> applicationClerkSharedData(CLERKCOUNT, 0);	//This can be used by the customer to pass SSN
std::vector<int> pictureClerkSharedDataSSN(CLERKCOUNT,0); //This can be used by the customer to pass SSN
std::vector<int> pictureClerkSharedDataPicture(CLERKCOUNT,0); // This can be used by the customer to pass acceptance of the picture
std::vector<int> passportClerkSharedDataSSN(CLERKCOUNT, 0); //This can be used by the customer to pass SSN
std::vector<bool> applicationCompletion(CUSTOMERCOUNT, 0); //Used by passportCerkto verify that application has been completed
std::vector<bool> pictureCompletion(CUSTOMERCOUNT, 0); //Used by passportClerk to verify that picture has beeen completed 
std::vector<bool> passportCompletion(CUSTOMERCOUNT,0); // Used by cashier to verify that the passport is complete
std::vector<int> passportPunishment(CUSTOMERCOUNT, 0); //Used by passportClerk to punish bad people.
std::vector<int> cashierSharedDataSSN(CLERKCOUNT, 0); //This can be used by the customer to pass SSN
std::vector<int> cashierRejection(CUSTOMERCOUNT, 0); //Used by the cashier to reject customers.
std::vector<int> doneCompletely(CUSTOMERCOUNT, 0); //Used by customer to tell when done.
//
//End variables
/////////////////////////////////



//Utility Functions

// Utility function to pick the shortest line
//Customer has already chosen type of line to get in just needs to pick which line
//Assumptions: The caller has a lock for the given MVs
//Parameters: 
	//lineCount: a vector of the lineCount
	//clerkState: a vector of the clerkState
int pickShortestLine(std::vector<int>& pickShortestlineCount, std::vector<CLERKSTATE>& pickShortestclerkState){
	int myLine = -1;
	int lineSize = 1000;
	for(int i=0; i < CLERKCOUNT; i++){
		//If lineCount < lineSize and clerk is not on break
		if(pickShortestlineCount[i] < lineSize && pickShortestclerkState[i] != ONBREAK ){
			myLine = i;
			lineSize = pickShortestlineCount[i];
		}
	}
	return myLine;	//This is the shortest line
}//End pickShortestLine

//End Utility Functions




//Customer
// Customers enter the Passport Office with a completed application. 
// Their job is the following:
//    Get their application accepted by the ApplicationClerk
//    Get their official picture taken by the PictureClerk. Customers have a random percentage to not like their picture.
//    Get their "filed" application and picture recognized by the PassportClerk
//    Pay the Cashier once their documentation has been approved by the PassportClerk. 
		//Once a Customer has paid for their passport by the Cashier, they are done. 

// Customers can either get their application accepted first, or get their picture taken first. 
	//They make this decision randomly. 
	//Once they decide which clerk to go to first, they get in the shortest line for that clerk.

// Customers also have up to $1600 to help reduce the time required to get their passport completed.
	// Each clerk is willing to accept $500 to allow a Customer to move up in line. 
	// Customer who can pay move in front of any Customer that has not paid. 
	// HOWEVER, they do not move in front of any Customer that has also paid. 
	// Customer money is to be deterined randomly, in increments of $100, $600, $1100, and $1600.
void customerApplicationClerkInteraction(int SSN, int &money);//forward declaration//prolly not cleaner like this just thought it would be nice to implement after the main Customer thread.
void customerPictureClerkInteraction(int SSN, int money);
void customerPassportClerkInteraction(int SSN, int money);
void customerCashierInteraction(int SSN, int money);
void customerCheckOut(int SSN);
void Customer(int id){
	int SSN = id;
	int myLine = -1;
	int money = (rand()%4)*500 + 100;

	//Should I go to the applicationClerk first or get my picture taken first?
	if(true || rand() % 2){
		//Go to application clerk first
		customerApplicationClerkInteraction(SSN, money);
		customerCheckOut(SSN);
		customerPictureClerkInteraction(SSN, money);
	}
	else {
		//Go to the picture clerk first
		customerPictureClerkInteraction(SSN, money);
		customerApplicationClerkInteraction(SSN, money);
	}

	//This terminates the customer should go at end.
	customerCheckOut(SSN);

	while(passportCompletion[SSN] == 0) {
		customerPassportClerkInteraction(SSN, money);
	}
	while(doneCompletely[SSN] == 0) {
		customerCashierInteraction(SSN, money);
	}


return;
	//Here are the output Guidelines for the Customer
	if(false){
	printf("Customer %i has gotten in regular line for PictureClerk %i.\n", SSN, myLine);
	printf("Customer %i has gotten in regular line for PassportClerk %i.\n", SSN, myLine);
	printf("Customer %i has gotten in regular line for Cashier %i.\n", SSN, myLine);
	printf("Customer %i has gotten in bribe line for PictureClerk %i.\n", SSN, myLine);
	printf("Customer %i has gotten in bribe line for PassportClerk %i.\n", SSN, myLine);
	printf("Customer %i has gotten in bribe line for Cashier %i.\n", SSN, myLine);
	printf("Customer %i has given SSN %i to PictureClerk %i.\n", SSN, SSN, myLine);
	printf("Customer %i has given SSN %i to PassportClerk %i.\n", SSN, SSN, myLine);
	printf("Customer %i has given SSN %i to Cashier %i.\n", SSN, SSN, myLine);
	printf("Customer %i does not like their picture from PictureClerk %i.\brn", SSN, myLine);
	printf("Customer %i does like their picture from PictureClerk %i.\n", SSN, myLine);
	printf("Customer %i has gone to PassportClerk %i too soon. They are going to the back of the line.\n", SSN, myLine);
	printf("Customer %i has gone to Cashier %i too soon. They are going to the back of the line.\n", SSN, myLine);
	printf("Customer %i has given Cashier %i $100.", SSN, myLine);
	printf("Customer %i is going outside the PassportOffice because there is a Senator present.", SSN);
	}

}//End Customer

//To tell the manager they did a great job and let him know we're done.
void customerCheckOut(int SSN){
	managerLock->Acquire();
	checkedOutCount++;
	managerLock->Release();
	printf("Customer %i is leaving the Passport Office.\n", SSN);
	currentThread->Finish();
}

//The Customer's Interaction with the applicationClerk
//    Get their application accepted by the ApplicationClerk
void customerApplicationClerkInteraction(int SSN, int &money){
	int myLine = -1;
	bool bribe = (money > 500) && (rand()%2);
	//I have decided to go to the applicationClerk

	//I should acquire the line lock
	applicationClerkLineLock->Acquire();
	//lock acquired

	//Can I go to counter, or have to wait? Should i bribe?
	//Pick shortest line with clerk not on break
	//Should i get in the regular line else i should bribe?
	if(!bribe){ //Get in regular line
		myLine = pickShortestLine(applicationClerkLineCount, applicationClerkState);
	}else{ //get in bribe line
		myLine = pickShortestLine(applicationClerkBribeLineCount, applicationClerkState);
	}
	
	//I must wait in line
	if(applicationClerkState[myLine] == BUSY){
		if(!bribe){
			applicationClerkLineCount[myLine]++;
			printf("Customer %i has gotten in regular line for ApplicationClerk %i.\n", SSN, myLine);
			applicationClerkLineCV[myLine]->Wait(applicationClerkLineLock);
			applicationClerkLineCount[myLine]--;
		}else{
			applicationClerkBribeLineCount[myLine]++;
			printf("Customer %i has gotten in bribe line for ApplicationClerk %i.\n", SSN, myLine);
			applicationClerkBribeLineCV[myLine]->Wait(applicationClerkLineLock);
			money -= 500;
			applicationClerkBribeLineCount[myLine]--;
		}
	}
printf("Hmm");
	//Clerk is AVAILABLE
	applicationClerkState[myLine] = BUSY;
	applicationClerkLineLock->Release();
	//Lets talk to clerk
	applicationClerkLock[myLine]->Acquire();
printf("Haqrd");
	//Give my data to my clerk
	//We already have a lock so put my SSN in applicationClerkSharedData
	applicationClerkSharedData[myLine] = SSN;
	printf("Customer %i has given SSN %i to ApplicationClerk %i.\n", SSN, SSN, myLine);
	applicationClerkCV[myLine]->Signal(applicationClerkLock[myLine]);
	//Wait for clerk to do their job
	applicationClerkCV[myLine]->Wait(applicationClerkLock[myLine]);
	
	//Done
	applicationClerkLock[myLine]->Release();
	//Done Return
	return;
}//End customerApplicationClerkInteraction


//The Customer's Interaction with the pictureClerk
//Get their picture accepted by the pictureClerk
void customerPictureClerkInteraction(int SSN, int money){
	int myLine = -1;
	bool bribe = (money > 500) && (rand()%2);

	//I should acquire the line lock
	pictureClerkLineLock->Acquire();
	//lock acquired

	//Can I go to counter, or have to wait? Should i bribe?
	//Pick shortest line with clerk not on break
	//Should i get in the regular line else i should bribe?
	if(!bribe){ //Get in regular line
		myLine = pickShortestLine(pictureClerkLineCount, pictureClerkState);
	}else{ //get in bribe line
		myLine = pickShortestLine(pictureClerkBribeLineCount, pictureClerkState);
	}
	
	//I must wait in line
	if(pictureClerkState[myLine] == BUSY){
		if(!bribe){
			pictureClerkLineCount[myLine]++;
			printf("Customer %i has gotten in regular line for PictureClerk %i.\n", SSN, myLine);
			pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock);
			pictureClerkLineCount[myLine]--;
		}else{
			pictureClerkBribeLineCount[myLine]++;
			printf("Customer %i has gotten in bribe line for PictureClerk %i.\n", SSN, myLine);
			pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock);
			money -= 500;
			pictureClerkBribeLineCount[myLine]--;
		}
	}
	//Clerk is AVAILABLE
	pictureClerkState[myLine] = BUSY;
	pictureClerkLineLock->Release();

	//Lets talk to clerk
	pictureClerkLock[myLine]->Acquire();
	//Give my data to my clerk
	//We already have a lock so put my SSN in pictureClerkSharedData
	pictureClerkSharedDataSSN[myLine] = SSN;
	printf("Customer %i has given SSN %i to PictureClerk %i.\n", SSN, SSN, myLine);


	pictureClerkCV[myLine]->Signal(pictureClerkLock[myLine]);
	pictureClerkCV[myLine]->Wait(pictureClerkLock[myLine]);
	//Wait for clerk to take the picture
	while(pictureClerkSharedDataPicture[myLine] == 0) {
		if(rand()%10 > 7) {
			printf("Customer %i does not like their picture from PictureClerk %i.\n", SSN, myLine);
			pictureClerkSharedDataPicture[myLine] = 0;
			pictureClerkCV[myLine]->Signal(pictureClerkLock[myLine]);
			//Wait for clerk to take the picture
			pictureClerkCV[myLine]->Wait(pictureClerkLock[myLine]);
		}
		else {
			printf("Customer %i does like their picture from PictureClerk %i.\n", SSN, myLine);
			pictureClerkSharedDataPicture[myLine] = 1;
			pictureClerkCV[myLine]->Signal(pictureClerkLock[myLine]);
			//Wait for clerk to take the picture
			pictureClerkCV[myLine]->Wait(pictureClerkLock[myLine]);
			break;
		}
	}
	pictureClerkCV[myLine]->Signal(pictureClerkLock[myLine]);
	pictureClerkLock[myLine]->Release();
	//Done Return
	return;
}//End customerPictureClerkInteraction

void customerPassportClerkInteraction(int SSN, int money){
	int myLine = -1;
	bool bribe = (money > 500) && (rand()%2);

	//I should acquire the line lock
	passportClerkLineLock->Acquire();
	//lock acquired

	//Can I go to counter, or have to wait? Should i bribe?
	//Pick shortest line with clerk not on break
	//Should i get in the regular line else i should bribe?
	if(!bribe){ //Get in regular line
		myLine = pickShortestLine(passportClerkLineCount, passportClerkState);
	}else{ //get in bribe line
		myLine = pickShortestLine(passportClerkBribeLineCount, passportClerkState);
	}
	
	//I must wait in line
	if(passportClerkState[myLine] == BUSY){
		if(!bribe){
			passportClerkLineCount[myLine]++;
			printf("Customer %i has gotten in regular line for PassportClerk %i.\n", SSN, myLine);
			passportClerkLineCV[myLine]->Wait(passportClerkLineLock);
			passportClerkLineCount[myLine]--;
		}else{
			passportClerkBribeLineCount[myLine]++;
			printf("Customer %i has gotten in bribe line for PassportClerk %i.\n", SSN, myLine);
			passportClerkBribeLineCV[myLine]->Wait(passportClerkLineLock);
			money -= 500;
			passportClerkBribeLineCount[myLine]--;
		}
	}
	//Clerk is AVAILABLE
	passportClerkState[myLine] = BUSY;
	passportClerkLineLock->Release();

	//Lets talk to clerk
	passportClerkLock[myLine]->Acquire();
	//Give my data to my clerk
	//We already have a lock so put my SSN in passportClerkSharedData
	passportClerkSharedDataSSN[myLine] = SSN;
	printf("Customer %i has given SSN %i to PassportClerk %i.\n", SSN, SSN, myLine);
	passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);
	//Wait for clerk to do their job
	passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);
	if(passportPunishment[SSN] == 1) {
		for(int i = 0; i < rand()%901 + 100; i++ ) { currentThread->Yield(); }
		return;
	}
	passportPunishment[SSN] = 0;
	//Done
	applicationClerkLock[myLine]->Release();
	//Done Return
	return;



}//End of customerPassportClerkInteraction


void customerCashierInteraction(int SSN, int money){
	int myLine = -1;
	bool bribe = (money > 500) && (rand()%2);

	//I should acquire the line lock
	cashierLineLock->Acquire();
	//lock acquired

	//Can I go to counter, or have to wait? Should i bribe?
	//Pick shortest line with clerk not on break
	//Should i get in the regular line else i should bribe?
	if(!bribe){ //Get in regular line
		myLine = pickShortestLine(cashierLineCount, cashierState);
	}else{ //get in bribe line
		myLine = pickShortestLine(cashierBribeLineCount, cashierState);
	}
	
	//I must wait in line
	if(cashierState[myLine] == BUSY){
		if(!bribe){
			cashierLineCount[myLine]++;
			printf("Customer %i has gotten in regular line for Cashier %i.\n", SSN, myLine);
			cashierLineCV[myLine]->Wait(cashierLineLock);
			cashierLineCount[myLine]--;
		}else{
			cashierBribeLineCount[myLine]++;
			printf("Customer %i has gotten in bribe line for Cashier %i.\n", SSN, myLine);
			cashierBribeLineCV[myLine]->Wait(cashierLineLock);
			money -= 500;
			cashierBribeLineCount[myLine]--;
		}
	}
	//Clerk is AVAILABLE
	cashierState[myLine] = BUSY;
	cashierLineLock->Release();

	//Lets talk to clerk
	cashierLock[myLine]->Acquire();
	//Give my data to my clerk
	//We already have a lock so put my SSN in cashierSharedData
	cashierSharedDataSSN[myLine] = SSN;
	pictureClerkSharedDataPicture[myLine] = 0;
	printf("Customer %i has given SSN %i to Cashier %i.\n", SSN, SSN, myLine);
	cashierCV[myLine]->Signal(cashierLock[myLine]);
	//Wait for clerk check completion


	cashierCV[myLine]->Wait(cashierLock[myLine]);
	if(passportPunishment[SSN] == 1) {
		for(int i = 0; i < rand()%901 + 100; i++ ) { currentThread->Yield(); }
	}
	//Done
	applicationClerkLock[myLine]->Release();
	//Done Return
	return;



}//End of customerCashierInteraction



//Senator: 
// Senators, being special people who need their privacy and security, get to use the Passport Office all by themselves. 
	// When a Senator shows up, they wait until all Customers currently being serviced are handled. 
	// This means only the Customers currently with a Clerk are to be handled. 
	// All other Customers must "leave" the Passport Office. 
	// Customers must remember the type of line they are in, but they have to get back in line once the Senator leaves.
	// Once the current Customers have been serviced, the Senator gets to go to each Clerk type and get their passport. 
	// Any Cusotmer showing up when a Senator is present must wait "outside" - a different line altogether.
// Once a Senator is finished, all waiting Customers get to proceed as normal. 
void Senator(int id){
	int SSN = id;
	int myLine = -1;


//Here are the output Guidelines for the Senator
	if(false){
	printf("Senator %i has gotten in regular line for ApplicationClerk %i.\n", SSN, myLine);
	printf("Senator %i has gotten in regular line for PictureClerk %i.\n", SSN, myLine);
	printf("Senator %i has gotten in regular line for PassportClerk %i.\n", SSN, myLine);
	printf("Senator %i has gotten in regular line for Cashier %i.\n", SSN, myLine);
	printf("Senator %i has given SSN %i to ApplicationClerk %i.\n", SSN, SSN, myLine);
	printf("Senator %i has given SSN %i to PictureClerk %i.\n", SSN, SSN, myLine);
	printf("Senator %i has given SSN %i to PassportClerk %i.\n", SSN, SSN, myLine);
	printf("Senator %i has given SSN %i to Cashier %i.\n", SSN, SSN, myLine);
	printf("Senator %i does not like their picture from PictureClerk %i.\n", SSN, myLine);
	printf("Senator %i does like their picture from PictureClerk %i.\n", SSN, myLine);
	printf("Senator %i has gone to PassportClerk %i too soon. They are going to the back of the line.\n", SSN, myLine);
	printf("Senator %i has gone to Cashier %i too soon. They are going to the back of the line.\n", SSN, myLine);
	printf("Senator %i has given Cashier %i $100.", SSN, myLine);
	printf("Senator %i is leaving the Passport Office.", SSN);
	}

} //End Senator



//ApplicationClerk - an application clerk accepts a completed Application. 
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
// This is determined by a random number of 'currentThread->Yield() calls - the number is to vary from 20 to 100.
void applicationClerkcheckAndGoOnBreak(int myLine); //Too many of these forward declarations
void ApplicationClerk(int id){
	int myLine = id;
	int money = 0;
	int customerFromLine;//0 no line, 1 bribe line, 3 regular line
//Keep running
	while(true){
		applicationClerkLineLock->Acquire();

		//If there is someone in my bribe line
		if(applicationClerkBribeLineCount[myLine] > 0){
			customerFromLine = 1;
			applicationClerkBribeLineCV[myLine]->Signal(applicationClerkLineLock);
			applicationClerkState[myLine] = BUSY;
		}else if(applicationClerkLineCount[myLine] > 0){//if there is someone in my regular line
			customerFromLine = 2;
			applicationClerkLineCV[myLine]->Signal(applicationClerkLineLock);
			applicationClerkState[myLine] = BUSY;
		}else{
			//No Customers
			//Go on break if there is another clerk
			customerFromLine = 0;
			applicationClerkcheckAndGoOnBreak(myLine);
		}

		//Should only do this when we have a customer...
		if(customerFromLine != 0){
			printf("ApplicationClerk %i has signalled a Customer to come to their counter.\n", myLine);
			applicationClerkLock[myLine]->Acquire();
			applicationClerkLineLock->Release();
			//wait for customer data
			applicationClerkCV[myLine]->Wait(applicationClerkLock[myLine]);
			//Customer Has given me their SSN?
			//And I have a lock
			int customerSSN = applicationClerkSharedData[myLine];
			
			//Customer from bribe line? //maybe should be separate signalwait  ehh?
			if(customerFromLine == 1){
				money += 500;
				printf("ApplicationClerk %i has received $500 from Customer %i.\n", myLine, customerSSN);
				currentThread->Yield();//Just to change things up a bit.
			}
			

			printf("ApplicationClerk %i has received SSN %i from Customer %i.\n", myLine, customerSSN, customerSSN);
			
			//Signal Customer that I'm Done.
			applicationClerkCV[myLine]->Signal(applicationClerkLock[myLine]);
			applicationClerkLock[myLine]->Release();

			//yield for filing time
			for(int i = 0; i < rand()%81 + 20; i++) { currentThread->Yield(); }
			
			//TODO: NEED TO ACQUIRE A LOCK FOR THIS!!
			applicationCompletion[customerSSN] = 1;
			printf("ApplicationClerk %i has recorded a completed application for Customer %i.\n", myLine, customerSSN);
		}//end if have customer

	}

}//End ApplicationClerk

//Utility for applicationClerk to gon on brak
// Assumptions: called with clerkLineLock
void applicationClerkcheckAndGoOnBreak(int myLine){
	//Only go on break if there is at least one other clerk
	bool freeOrAvailable = false;
	for(int i = 0; i < CLERKCOUNT; i++){
		if(i != myLine && ( applicationClerkState[i] == AVAILABLE || applicationClerkState[i] == BUSY ) ){
			freeOrAvailable = true;
			break;
		}
	}
	//There is at least one clerk...go on a break.
	if(freeOrAvailable){
		applicationClerkState[myLine] = ONBREAK;
		printf("ApplicationClerk %i is going on break.\n", myLine);
		applicationClerkBreakCV->Wait(applicationClerkLineLock);
		applicationClerkState[myLine] = BUSY;
		printf("ApplicationClerk %i is coming off break.\n", myLine);
	}else{
		//If everyone is on break...
		//applicationClerkState[myLine] = AVAILABLE;
		applicationClerkLineLock->Release();
		//Should we go to sleep?
		managerLock->Acquire();//Do we really need to acquire a lock for this?
		if(checkedOutCount == CUSTOMERCOUNT){managerLock->Release(); currentThread->Finish();}
		managerLock->Release();//Guess not
		currentThread->Yield();
		applicationClerkLineLock->Acquire();
	}
	//applicationClerkState[myLine] = AVAILABLE;
}





//PictureClerk - they take pictures of Customers. 
	//Customers can turn in their application or get their picture taken in any order. 
	//This is to be randomly determined by each customer when they are executing. 
//
// PictureClerks "take" the picture of a Customer. Customers are "shown" their picture. 
	// They have some percentage chance of not liking their picture.
	// You get to decide what that percentage is. It cannot be zero. 
	// Once a Customer accepts their picture, this "official" picture is filed by the PictureClerk.
	// Customers are told when this has been completed.
// Any money received from Customers wanting to move up in line must be added to the PictureClerk received money amount.
// PictureClerks go on break if they have no Customers in their line.
// There is always a delay in an accepted picture being "filed". 
// This is determined by a random number of 'currentThread->Yield() calls - the number is to vary from 20 to 100.
void pictureClerkcheckAndGoOnBreak(int myLine);
void PictureClerk(int id){
		int myLine = id;
		int money = 0;
		int identifier = -1; //TODO: REMOVE THIS!!!
		int customerFromLine;//0 no line, 1 bribe line, 3 regular line
		//Keep running
		while(true){
	
			pictureClerkLineLock->Acquire();

			//If there is someone in my bribe line
			if(pictureClerkBribeLineCount[myLine] > 0){
				customerFromLine = 1;
				money += 500;
				printf("PictureClerk %i has received $500 from Customer %i.\n", myLine, identifier);
				pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock);
				pictureClerkState[myLine] = BUSY;
			}else if(pictureClerkLineCount[myLine] > 0){//if there is someone in my regular line
				customerFromLine = 2;
				pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock);
				pictureClerkState[myLine] = BUSY;
			}else{
				//Go on a break!
				customerFromLine = 0;
				pictureClerkcheckAndGoOnBreak(myLine);
			}

			//Should only do this when we are BUSY? We have a customer...
			if(customerFromLine != 0){
				pictureClerkSharedDataPicture[myLine] = 0;
				printf("PictureClerk %i has signalled a Customer to come to their counter.\n", myLine);
				pictureClerkLock[myLine]->Acquire();
				pictureClerkLineLock->Release();
				//wait for customer data
				pictureClerkCV[myLine]->Wait(pictureClerkLock[myLine]);
				//Customer Has given me their SSN?
				//And I have a lock
				int customerSSN = pictureClerkSharedDataSSN[myLine];
				printf("PictureClerk %i has received SSN %i from Customer %i.\n", myLine, customerSSN, customerSSN);
				pictureClerkSharedDataPicture[myLine] = 0;
				while(pictureClerkSharedDataPicture[myLine] == 0) {
					//Taking picture
					printf("PictureClerk %i has taken a picture of Customer %i.\n", myLine, customerSSN);
					//Signal Customer that I'm Done and show them the picture. Then wait for response.
					pictureClerkCV[myLine]->Signal(pictureClerkLock[myLine]);
					pictureClerkCV[myLine]->Wait(pictureClerkLock[myLine]);
					if(pictureClerkSharedDataPicture[myLine] == 0)  {
						printf("PictureClerk %i has has been told that Customer %i does not like their picture.\n", myLine, customerSSN);
					}

				}
				printf("PictureClerk %i has has been told that Customer %i does like their picture.\n", myLine, customerSSN);
				//Yield before submitting.
				for(int i = 0; i < rand()%81 + 20; i++) { currentThread->Yield(); }
				printf("PictureClerk %i has recorded a completed picture for Customer %i.\n", myLine, customerSSN);
				pictureCompletion[customerSSN] = 1;
				//Signal Customer that I'm Done.
				pictureClerkCV[myLine]->Signal(pictureClerkLock[myLine]);
				pictureClerkCV[myLine]->Wait(pictureClerkLock[myLine]);

				pictureClerkLock[myLine]->Release();
			}
	
		}

}//End PictureClerk

/*//Utility for pictureClerk to go on brak
// Assumptions: called with clerkLineLock
void pictureClerkcheckAndGoOnBreak(int myLine){
	//Only go on break if there is at least one other clerk
	bool freeOrAvailable = false;
	for(int i = 0; i < CLERKCOUNT; i++){
		if(i != myLine && ( pictureClerkState[i] == AVAILABLE || pictureClerkState[i] == BUSY ) ){
			freeOrAvailable = true;
			break;
		}
	}
	//There is at least one clerk...go on a break.
	if(freeOrAvailable){
		pictureClerkState[myLine] = ONBREAK;
		printf("PictureClerk %i is going on break.\n", myLine);
		pictureClerkBreakCV->Wait(pictureClerkLineLock);
		printf("PictureClerk %i is coming off break.\n", myLine);
	}else{
		currentThread->Yield();
	}
	pictureClerkState[myLine] = AVAILABLE;
}*/

//Utility for applicationClerk to gon on brak
// Assumptions: called with clerkLineLock
void pictureClerkcheckAndGoOnBreak(int myLine){
	//Only go on break if there is at least one other clerk
	bool freeOrAvailable = false;
	for(int i = 0; i < CLERKCOUNT; i++){
		if(i != myLine && ( pictureClerkState[i] == AVAILABLE || pictureClerkState[i] == BUSY ) ){
			freeOrAvailable = true;
			break;
		}
	}
	//There is at least one clerk...go on a break.
	if(freeOrAvailable){
		pictureClerkState[myLine] = ONBREAK;
		printf("PictureClerk %i is going on break.\n", myLine);
		pictureClerkBreakCV->Wait(pictureClerkLineLock);
		pictureClerkState[myLine] = BUSY;
		printf("PictureClerk %i is coming off break.\n", myLine);
	}else{
		//If everyone is on break...
		//applicationClerkState[myLine] = AVAILABLE;
		pictureClerkLineLock->Release();
		//Should we go to sleep?
		managerLock->Acquire();//Do we really need to acquire a lock for this?
		if(checkedOutCount == CUSTOMERCOUNT){managerLock->Release(); currentThread->Finish();}
		managerLock->Release();//Guess not
		currentThread->Yield();
		pictureClerkLineLock->Acquire();
	}
	//applicationClerkState[myLine] = AVAILABLE;
}








// PassportClerk - they check that a Customer has filed their completed application and has an official picture taken.
//
// PassportClerks will "certify" a Customer ONLY if the Customer has a filed application and picture. 
	// If a Customer shows up to the PassportClerk BEFORE both documenets are filed,
	// they they (the Customer) are punished by being forec to wait for some arbitrary amount of time.
	// This is to be from 100 to 1000 currentThread->Yield() calls.
	// After these calls are completed, ,the Customer goes to the back of the PassportClerk line.
	// NOTE It takes time for a PassportClerk to "record" a Customer's completed documents. 
void PassportClerk(int id){
	int myLine = id;
	int money = 0;
	int identifier = -1; //TODO: REMOVE THIS SORRY FOR ADDING THESE
	//Keep running
	while(true){

		passportClerkLineLock->Acquire();

		//If there is someone in my bribe line
		if(passportClerkBribeLineCount[myLine] > 0){
			money += 500;
			passportClerkBribeLineCV[myLine]->Signal(passportClerkLineLock);
			passportClerkState[myLine] = BUSY;
		}else if(passportClerkLineCount[myLine] > 0){//if there is someone in my regular line
			passportClerkLineCV[myLine]->Signal(passportClerkLineLock);
			passportClerkState[myLine] = BUSY;
		}else{
			//eventually go on break //for now //?
			passportClerkState[myLine] = AVAILABLE;
		}

		//Should only do this when we are BUSY? We have a customer...
		if(passportClerkState[myLine] == BUSY){
			printf("PassportClerk %i has signalled a Customer to come to their counter.\n", myLine);
			passportClerkLock[myLine]->Acquire();
			passportClerkLineLock->Release();
			//wait for customer data
			passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);
			//Customer Has given me their SSN?
			//And I have a lock
			int customerSSN = passportClerkSharedDataSSN[myLine];
			printf("PassportClerk %i has received SSN %i from Customer %i.\n", myLine, customerSSN, customerSSN);
			
			//Do my job - customer waiting
			if(!(applicationCompletion[customerSSN] == 1 && pictureCompletion[customerSSN] == 1)) {
				passportPunishment[customerSSN] = 1;
				printf("PassportClerk %i has determined that Customer %i does not have both their application and picture completed.\n", myLine, identifier);
				passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);
				passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);
			}
			else {
				passportPunishment[customerSSN] = 0;
				printf("PassportClerk %i has determined that Customer %i has both their application and picture completed.\n", myLine, identifier);
				passportCompletion[customerSSN] = true;
				for(int i = 0; i < rand()%81 + 20; i++) { currentThread->Yield(); }
				printf("PassportClerk %i has recorded Customer %i passport documentation.\n", myLine, identifier);
			}
			//Signal Customer that I'm Done.
			passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);
			//passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);//Idk if this is needed...
			passportClerkLock[myLine]->Release();
		}

	}



	//Here are the output Guidelines for the PassportClerk
	if(false){
	printf("PassportClerk %i is going on break.\n", myLine);
	printf("PassportClerk %i is coming off break.\n", myLine);
	}

}//End PassportClerk









// Cashier - Once a PassportClerk has "certified" that a proper application has been completed,
	// the Cashier will accept the $100 necessary to pay for the new passport.
//
// Cashiers take the $100 passport fee from Customer 
	// - ONLY after they see a "certification" by the PassportClerk.
	// If a Customer tries to pay for their passport BEFORE they are certified by the PassportClerk,
	//  then they are punished by being forced to wait for some arbitrary amount of time.
	// This is to be from 100 to 1000 curerntThread->Yield() calls. 
	// After these calls are completed, the Customer goes to the back of a Cashier line.
// Once a Cashier accepts a Customers money, they "provide" the completed passport.
	// The Cashier records that a passport has been given to the Customer, 
	// so that multiple passports cannot be handed out to the same Customer.
//Any money received from Customers. 
	// Whether the standard application fee,
	// or from those Customers wanting to move up in line must be added to the Cashier received money amount.
// Cashiers go on break if they have no Customers in their line
void Cashier(int id){
	
	int myLine = id;
	int money = 0;
	int identifier = -1; //TODO: REMOVE THIS SORRY FOR ADDING THESE
	//Keep running
	while(true){

		cashierLineLock->Acquire();

		//If there is someone in my bribe line
		if(cashierBribeLineCount[myLine] > 0){
			money += 500;
			cashierBribeLineCV[myLine]->Signal(cashierLineLock);
			cashierState[myLine] = BUSY;
		}else if(cashierLineCount[myLine] > 0){//if there is someone in my regular line
			cashierLineCV[myLine]->Signal(cashierLineLock);
			cashierState[myLine] = BUSY;
		}else{
			//eventually go on break //for now //?
			cashierState[myLine] = AVAILABLE;
		}

		//Should only do this when we are BUSY? We have a customer...
		if(cashierState[myLine] == BUSY){
			printf("Cashier %i has signalled a Customer to come to their counter.\n", myLine);
			cashierLock[myLine]->Acquire();
			cashierLineLock->Release();
			//wait for customer data
			cashierCV[myLine]->Wait(cashierLock[myLine]);
			//Customer Has given me their SSN?
			//And I have a lock
			int customerSSN = cashierSharedDataSSN[myLine];
			printf("Cashier %i has received SSN %i from Customer %i.\n", myLine, customerSSN, customerSSN);
			
			//Do my job - customer waiting
			if(passportCompletion[customerSSN] == 0) {
				printf("Cashier %i has received the $100 from Customer%i before certification. They are to go to the back of my line.\n", myLine, customerSSN);
				cashierRejection[customerSSN] = 1;
			}
			else {
				cashierRejection[customerSSN] = 0;
				printf("Cashier %i has verified that Customer %i has been certified by a PassportClerk.\n", myLine, customerSSN);
				//tell customer and wait to be paid
				cashierCV[myLine]->Signal(cashierLock[myLine]);
				cashierCV[myLine]->Wait(cashierLock[myLine]);
				printf("Cashier %i has received the $100 from Customer%i after certification.\n", myLine, customerSSN);

				printf("Cashier %i has provided Customer %i their completed passport.\n", myLine, customerSSN);

			}
			//Signal Customer that I'm Done.
			cashierCV[myLine]->Signal(cashierLock[myLine]);
			//cashierCV[myLine]->Wait(cashierLock[myLine]);//Idk if this is needed...
			cashierLock[myLine]->Release();
		}

	}






	//Here are the output Guidelines for the Cashier
	if(false){
	printf("Cashier %i has signalled a Customer to come to their counter.\n", myLine);
	printf("Cashier %i has received SSN %i from Customer %i.\n", myLine, identifier, identifier);
	printf("Cashier %i has verified that Customer %i has been certified by a PassportClerk.\n", myLine, identifier);
	printf("Cashier %i has received the $100 from Customer %i after certification.\n", myLine, identifier);
	printf("Cashier %i has recorded the $100 from Customer %i before certification. They are to go to the back of my line.\n", myLine, identifier);
	printf("Cashier %i has provided Customer %i their completed passport.\n", myLine, identifier);
    printf("Cashier %i has recorded that Customer %i has been given their completed passport.\n", myLine, identifier);
	printf("Cashier %i is going on break.\n", myLine);
	printf("Cashier %i is coming off break.\n", myLine);
	}



}//End Cashier




// Managers tell the various Clerks when to start working, when lines get too long. 
//
// Clerks go on break when they have no one in their line.
	// Managers are responsible for waking up Clerks when more than 3 Customers get in any particular line.
// Managers are also responsible for keeping track of how much money the office has made.
	// You are to print the total received from each clerk type, and a total from all clerks.
	// This is to be printed on a fairly regular basis.
void managerCheckandWakupClerks();//Forward declaration..?
void checkEndOfDay();
void Manager(int id){

	//Untill End of Simulation
	while(true){


		//Check Lines Wake up Clerk if More than 3 in a line.
		//Check ApplicationClerk
		managerCheckandWakupClerks();

		//Check if all the customers are gone and let all the clerks go home
		checkEndOfDay();


		currentThread->Yield();//Let someone else get the CPU
	}



	//Here are the output Guidelines for the Manager
	if(false){
	int identifier = -1;
	printf("Manager has woken up an PassportClerk.\n");
	printf("Manager has woken up an Cashier.\n");
	printf("Manager has counted a total of $%i for ApplicationClerks.\n", identifier);
	printf("Manager has counted a total of $%i for PictureClerks.\n", identifier);
	printf("Manager has counted a total of $%i for PassportClerks.\n", identifier);
	printf("Manager has counted a total of $%i for Cashiers.\n", identifier);
	printf("Manager has counted a total of $%i for the passport office.\n", identifier);
	}

}//End Manager


//This will put the clerks and the manager to sleep so everyone can go to sleep and nachos can clean up
void checkEndOfDay(){
	managerLock->Acquire();
	if (checkedOutCount == CUSTOMERCOUNT){
		//printf("DEBUG: MANAGER: END OF DAY!\n");
		//All the customers are gone
		//Lets all go to sleep
		managerLock->Release();
		currentThread->Finish();
	}
	managerLock->Release();
}

//Utilities for Manager

// managerCheckandWakeupCLERK
// checks if a line has more than 3 customers... 
// if so, signals a clerk on break
// Returns true if there was asleeping clerk and needed to wake one up
bool managerCheckandWakeupCLERK(Lock* managerCWCLineLock, std::vector<int>& managerCWClineCount, std::vector<CLERKSTATE>& managerCWCState, Condition* managerCWCBreakCV, int managerCWCount){
	bool wakeUp = false;//should we wake up a clerk?
	bool asleep = false;//is any clerk asleep?
	managerCWCLineLock->Acquire();
	for(int i = 0; i < managerCWCount; i++){
		if(managerCWCState[i] == ONBREAK)
			asleep = true;
		if(managerCWClineCount[i] > 3)
			wakeUp = true;
	}
	if(wakeUp && asleep){managerCWCBreakCV->Signal(managerCWCLineLock);}
	managerCWCLineLock->Release();
	return asleep && wakeUp;
}


//managerCheckandWakupClerks()
//Checks all types of clerks for lines longer than 3 and wakes up a sleaping clerk if there is one
void managerCheckandWakupClerks(){
	//Check Application Clerks
	if(managerCheckandWakeupCLERK(applicationClerkLineLock, applicationClerkLineCount, applicationClerkState, applicationClerkBreakCV, CLERKCOUNT)){
		printf("Manager has woken up an ApplicationClerk.\n");
	}

	//Check Picture Clerks
	if(managerCheckandWakeupCLERK(pictureClerkLineLock, pictureClerkLineCount, pictureClerkState, pictureClerkBreakCV, CLERKCOUNT)){
		printf("Manager has woken up a PictureClerk.\n");
	}

	//TODO CHECK OTHER CLERKS!
}

//End Manager functions
///////////////////////////////








//This runs the simulation
void Part2TestSuit(){

	printf("DEBUG: Starting Passport Office Simulation with %i Customers\n", CUSTOMERCOUNT);
	printf("DEBUG: Starting Count: %i\n", checkedOutCount);


	//Initialize dynamic variables
	for(int i = 0; i < CLERKCOUNT; i++){
		applicationClerkLock.push_back(new Lock("applicationClerkLock" + i));
		applicationClerkLineCV.push_back(new Condition("applicationClerkLineCV" + i));
		applicationClerkBribeLineCV.push_back(new Condition("applicationClerkBribeLineCV" + i));
		applicationClerkCV.push_back(new Condition("applicationClerkCV" + i));

		pictureClerkLock.push_back(new Lock("pictureClerkLock" + i));
		pictureClerkLineCV.push_back(new Condition("picutreClerkLineCV" + i));
		pictureClerkBribeLineCV.push_back(new Condition("pictureClerkBribeLineCV" + i));
		pictureClerkCV.push_back(new Condition("pictureClerkCV" + i));

		passportClerkLock.push_back(new Lock("passportClerkLock" + i));
		passportClerkLineCV.push_back(new Condition("picutreClerkLineCV" + i));
		passportClerkBribeLineCV.push_back(new Condition("passportClerkBribeLineCV" + i));
		passportClerkCV.push_back(new Condition("passportClerkCV" + i));
	}



	Thread *t;

	for(int i = 0; i < CUSTOMERCOUNT; i++){
		t = new Thread("Customer " + i);
		t->Fork(Customer, i);
	}
	
	for(int i = 0; i < CLERKCOUNT; i++){
		t = new Thread("ApplicationClerk " + i);
		t->Fork(ApplicationClerk, i);

		t = new Thread("PictureClerk " + i);
		t->Fork(PictureClerk, i);

		//t = new Thread("PassportClerk " + i);
		//t->Fork(PassportClerk, i);

		//t = new Thread("Cashier " + i);
		//t->Fork(Cashier, i);
	}
	
	t = new Thread("Manager");
	t->Fork(Manager, 0);

}










#endif//PROJ1_PART2_CC
