// Group 25
// Part2.cc

#ifndef PROJ1_PART2_CC
#define PROJ1_PART2_CC

#include "synch.h"
#include "list.h"
#include "vector"

//Settings Variables 
//TODO:These should be more dynamic
int CLERKCOUNT = 1;		//The number of clerks
int CUSTOMERCOUNT = 3; 	//Number of customers

//Globals or constants
enum CLERKSTATE {AVAILABLE, BUSY, ONBREAK};				//enum for the CLERKSTATE

///////////////////////////////////
//Initialize Locks, CVS, and Monitors?
//Locks
Lock *clerkLineLock = new Lock("clerkLineLock");		//The clerkLineLock //for testing
Lock *applicationClerkLineLock = new Lock("applicationClerkLineLock");	//The applicationClerkLineLock
std::vector<Lock*> clerkLock;	//for testing//shouldberemoved
std::vector<Lock*> applicationClerkLock;
//CVS
std::vector<Condition*> clerkLineCV;
std::vector<Condition*> clerkBribeLineCV;	//clerkCVs //for testing should be removed
std::vector<Condition*> clerkCV;

std::vector<Condition*> applicationClerkLineCV;
std::vector<Condition*> applicationClerkBribeLineCV;	//applicationClerk CVs
std::vector<Condition*> applicationClerkCV;
//States
std::vector<CLERKSTATE> clerkState(CLERKCOUNT, BUSY);	//clerkState
std::vector<CLERKSTATE> applicationClerkState(CLERKCOUNT, BUSY);	//applicationClerkState
std::vector<CLERKSTATE> pictureClerkState(CLERKCOUNT, BUSY);	//applicationClerkState
std::vector<CLERKSTATE> passportClerkState(CLERKCOUNT, BUSY);	//applicationClerkState
std::vector<CLERKSTATE> cashierState(CLERKCOUNT, BUSY);	//applicationClerkState
//LineCounts
std::vector<int> clerkLineCount(CLERKCOUNT, 0);			//clerkLineCount
std::vector<int> clerkBribeLineCount(CLERKCOUNT, 0);		//should be removed for testing only
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
void customerApplicationClerkInteraction(int SSN);//forward declaration//prolly not cleaner like this just thought it would be nice to implement after the main Customer thread.
void Customer(int id){
	int SSN = id;
	int myLine = -1;


	//Should I go to the applicationClerk first or get my picture taken first?
	if(true){
		//Go to application clerk
		customerApplicationClerkInteraction(SSN);
	}


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
	printf("Customer %i does not like their picture from PictureClerk %i.\n", SSN, myLine);
	printf("Customer %i does like their picture from PictureClerk %i.\n", SSN, myLine);
	printf("Customer %i has gone to PassportClerk %i too soon. They are going to the back of the line.\n", SSN, myLine);
	printf("Customer %i has gone to Cashier %i too soon. They are going to the back of the line.\n", SSN, myLine);
	printf("Customer %i has given Cashier %i $100.", SSN, myLine);
	printf("Customer %i is going outside the PassportOffice because there is a Senator present.", SSN);
	printf("Customer %i is leaving the Passport Office.", SSN);
	}

}//End Customer

//TODO: FIX THIS!!!!! JUST STARTED!!!!!!!!!!!
//The Customer's Interaction with the applicationClerk
//    Get their application accepted by the ApplicationClerk
void customerApplicationClerkInteraction(int SSN){
	int myLine = -1;
	bool bribe = false;	//TODO: should init this somehow
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
			applicationClerkBribeLineCount[myLine]--;
		}
	}
	//Clerk is AVAILABLE
	applicationClerkState[myLine] = BUSY;
	applicationClerkLineLock->Release();

	//Lets talk to clerk
	applicationClerkLock[myLine]->Acquire();
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
void ApplicationClerk(int id){
	int myLine = id;
	int identifier = -1; //TODO: Placeholder should be removed.

	//Keep running
	while(true){

		applicationClerkLineLock->Acquire();

		//If there is someone in my bribe line
		if(applicationClerkBribeLineCount[myLine] > 0){
			
			applicationClerkLineCV[myLine]->Signal(applicationClerkLineLock);
			applicationClerkState[myLine] = BUSY;
		}else if(applicationClerkLineCount[myLine] > 0){//if there is someone in my regular line
			applicationClerkBribeLineCV[myLine]->Signal(applicationClerkLineLock);
			applicationClerkState[myLine] = BUSY;
		}else{
			//eventually go on break //for now //?
			applicationClerkState[myLine] = AVAILABLE;
		}

		//Should only do this when we are BUSY? We have a customer...
		if(clerkState[myLine] == BUSY){
			printf("ApplicationClerk %i has signalled a Customer to come to their counter.\n", myLine);
			applicationClerkLock[myLine]->Acquire();
			applicationClerkLineLock->Release();
			//wait for customer data
			applicationClerkCV[myLine]->Wait(applicationClerkLock[myLine]);
			//Customer Has given me their SSN?
			//And I have a lock
			int customerSSN = applicationClerkSharedData[myLine];
			printf("ApplicationClerk %i has received SSN %i from Customer %i.\n", myLine, customerSSN, customerSSN);
			
			//Do my job - customer waiting

			printf("ApplicationClerk %i has recorded a completed application for Customer %i.\n", myLine, identifier);
			//Signal Customer that I'm Done.
			applicationClerkCV[myLine]->Signal(applicationClerkLock[myLine]);
			//applicationClerkCV[myLine]->Wait(applicationClerkLock[myLine]);//Idk if this is needed...
			applicationClerkLock[myLine]->Release();
		}

	}

	//Here are the output Guidelines for the ApplicationClerk
	if(false){
	printf("ApplicationClerk %i has received $500 from Customer %i.\n", myLine, identifier);
	printf("ApplicationClerk %i is going on break.\n", myLine);
	printf("ApplicationClerk %i is coming off break.\n", myLine);
	}
}//End ApplicationClerk






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
void PictureClerk(int id){
	int myLine = id;


	//Here are the output Guidelines for the ApplicationClerk
	if(false){
	int identifier = -1;
	printf("PictureClerk %i has signalled a Customer to come to their counter.\n", myLine);
	printf("PictureClerk %i has received SSN %i from Customer %i.\n", myLine, identifier, identifier);
	printf("PictureClerk %i has taken a picture of Customer %i.\n", myLine, identifier);
	printf("PictureClerk %i has has been told that Customer %i does not like their picture.\n", myLine, identifier);
	printf("PictureClerk %i has has been told that Customer %i does like their picture.\n", myLine, identifier);
	printf("PictureClerk %i is going on break.\n", myLine);
	printf("PictureClerk %i is coming off break.\n", myLine);
	}
}//End PictureClerk






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


	//Here are the output Guidelines for the PassportClerk
	if(false){
	int identifier = -1;
	printf("PassportClerk %i has signalled a Customer to come to their counter.\n", myLine);
	printf("PassportClerk %i has received SSN %i from Customer %i.\n", myLine, identifier, identifier);
	printf("PassportClerk %i has determined that Customer %i does not have both their application and picture completed.\n", myLine, identifier);
	printf("PassportClerk %i has determined that Customer %i has both their application and picture completed.\n", myLine, identifier);
	printf("PassportClerk %i has recorded Customer %i passport documentation.\n", myLine, identifier);
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




	//Here are the output Guidelines for the Cashier
	if(false){
	int identifier = -1;
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
void Manager(int id){



	//Here are the output Guidelines for the Cashier
	if(false){
	int identifier = -1;
	printf("Manager has woken up an ApplicationClerk.\n");
	printf("Manager has woken up an PictureClerk.\n");
	printf("Manager has woken up an PassportClerk.\n");
	printf("Manager has woken up an Cashier.\n");
	printf("Manager has counted a total of $%i for ApplicationClerks.\n", identifier);
	printf("Manager has counted a total of $%i for PictureClerks.\n", identifier);
	printf("Manager has counted a total of $%i for PassportClerks.\n", identifier);
	printf("Manager has counted a total of $%i for Cashiers.\n", identifier);
	printf("Manager has counted a total of $%i for the passport office.\n", identifier);
	}

}//End Manager






//The Customer thread
//This was used for testing and as an example should be removed eventually
void CustomerTest(int id){
	int SSN = id;	//The Customer's ID || SSN
	printf("Customer %i: Initialized.\n", SSN);

	clerkLineLock->Acquire();
	printf("Customer %i: clerkLineLock Acquired.\n", SSN);

	//Can I go to counter, or have to wait? Should i bribe?
	//Pick shortest line with clerk not on break
	int myLine = -1;
	if(false){//Should i pick the bribe line?
		myLine = pickShortestLine(clerkBribeLineCount, clerkState);
	}else{
		myLine = pickShortestLine(clerkLineCount, clerkState);
	}
	

	printf("Customer %i: picked line/clerk %i.\n", SSN, myLine);
	if(clerkState[myLine] == BUSY){
		//I must wait in line
		clerkLineCount[myLine]++;
		printf("Customer %i: Clerk busy, about to wait in line %i.\n", SSN, myLine);
		clerkLineCV[myLine]->Wait(clerkLineLock);
		printf("Customer %i: Clerk signalled, done waiting.\n", SSN);
		clerkLineCount[myLine]--;
	}
	//Clerk is AVAILABLE
	clerkState[myLine] = BUSY;
	clerkLineLock->Release();

	//For now just return
	clerkLock[myLine]->Acquire();
	clerkCV[myLine]->Signal(clerkLock[myLine]);
	clerkLock[myLine]->Release();
	printf("Customer %i: Done Returning.\n", SSN);
	return;
/*
	clerkLock[myLine]->Acquire();
	//Give my data to clerk
	clerkCV[myLine]->Signal(clerkLock[myLine]);
	//wait for clerk to do their job
	clerkCV[myLine]->Wait(clerkLock[myLine]);
	//read my data
	clerkCV[myLine]->Signal(clerkLock[myLine]);
	clerkLock[myLine]->Release();
*/
}//End Customer







//The Clerk thread
void ClerkTest(int whatLine){

	int myLine = whatLine;
	printf("Clerk %i: Initialized.\n", myLine);

	while(true){

		clerkLineLock->Acquire();
		//printf("Clerk %i: LineLock acquired.\n", myLine);

		if(false && clerkBribeLineCount[myLine] > 0){
			
			clerkBribeLineCV[myLine]->Signal(clerkLineLock);
			clerkState[myLine] = BUSY;
		}else if(clerkLineCount[myLine] > 0){
			printf("Clerk %i: %i Customers in line, signalling...\n", myLine, clerkLineCount[myLine]);
			clerkLineCV[myLine]->Signal(clerkLineLock);
			clerkState[myLine] = BUSY;
		}else{
			//eventually go on break //for now
			//printf("Clerk %i: AVAILABLE.\n", myLine);
			clerkState[myLine] = AVAILABLE;
		}
		//For now release lock and continue...?
		//remove this after testing.
		if(clerkState[myLine] == BUSY){
			clerkLock[myLine]->Acquire();
			clerkLineLock->Release();
			clerkCV[myLine]->Wait(clerkLock[myLine]);
			clerkLock[myLine]->Release();
			printf("Clerk Done continue\n");
			continue;
		}else{
			currentThread->Yield();
		}

		/*//Should only do this when we are BUSY?
		if(clerkState[myLine] == BUSY){
			clerkLock[myLine]->Acquire();
			clerkLineLock->Release();
			//wait for customer data
			clerkCV[myLine]->Wait(clerkLock[myLine]);
			//Do my job - customer waiting
			clerkCV[myLine]->Signal(clerkLock[myLine]);
			clerkCV[myLine]->Wait(clerkLock[myLine]);
			clerkLock[myLine]->Release();
		}*/

	}
}//End Clerk


//This runs the simulation
void Part2TestSuit(){

	//Initialize Variables
	for(int i = 0; i < CLERKCOUNT; i++){
		clerkLineCV.push_back(new Condition("clerkLineCV" + i));
		clerkBribeLineCV.push_back(new Condition("clerkLineCV" + i));
		clerkLock.push_back(new Lock("clerkLock" + i));
		clerkCV.push_back(new Condition("clerkCV" + i));
	}



	
	Thread *t;
	t = new Thread("Customer 0");
	t->Fork(CustomerTest, 0);
	t = new Thread("Customer 1");
	t->Fork(CustomerTest, 1);
	t = new Thread("Customer 2");
	t->Fork(CustomerTest, 2);

	t = new Thread("Clerk 0");
	t->Fork(ClerkTest, 0);

}










#endif//PROJ1_PART2_CC
