// Group 25
// Part2.h

#ifndef PROJ1_PART2_H
#define PROJ1_PART2_H

#include "synch.h"
#include "list.h"
#include "vector"

//Settings Variables
int CLERKCOUNT = 1;		//The number of clerks
int CUSTOMERCOUNT = 2; 	//Number of customers

//Globals or constants
enum CLERKSTATE {AVAILABLE, BUSY, ONBREAK};		//enum for the CLERKSTATE


//Initialize Locks, CVS, and Monitors?
Lock *clerkLineLock = new Lock("clerkLineLock");						//The clerkLineLock
std::vector<int> clerkLineCount(CLERKCOUNT, 0);				//clerkLineCount
std::vector<CLERKSTATE> clerkState(CLERKCOUNT, AVAILABLE);	//clerkState
std::vector<Condition*> clerkLineCV;
std::vector<Lock*> clerkLock;
std::vector<Condition*> clerkCV;
std::vector<int> clerkBribeLineCount(CLERKCOUNT, 0);
std::vector<Condition*> clerkBribeLineCV;

//The Customer thread
void Customer(int id){
	int SSN = id;	//The Customer's ID || SSN
	printf("Customer %i: Initialized.\n", SSN);
	clerkLineLock->Acquire();
	printf("Customer %i: clerkLineLock Acquired.\n", SSN);
	//Can I go to counter, or have to wait?
	//Pick shortest line with clerk not on break
	int myLine = -1;
	int lineSize = 1000;
	for(int i=0; i < CLERKCOUNT; i++){
		if(clerkLineCount[i] < lineSize && clerkState[i] != ONBREAK){
			myLine = i;
			lineSize = clerkLineCount[i];
		}
	}
	if(clerkState[myLine] == BUSY){
		//I must wait in line
		clerkLineCount[myLine]++;
		clerkLineCV[myLine]->Wait(clerkLineLock);
		clerkLineCount[myLine]--;
	}
	//Clerk is AVAILABLE
	clerkState[myLine] = BUSY;
	clerkLineLock->Release();

	//For now just return
	printf("Customer %i: Done Returning.\n", SSN);
	return;
	clerkLock[myLine]->Acquire();
	//Give my data to clerk
	clerkCV[myLine]->Signal(clerkLock[myLine]);
	//wait for clerk to do their job
	clerkCV[myLine]->Wait(clerkLock[myLine]);
	//read my data
	clerkCV[myLine]->Signal(clerkLock[myLine]);
	clerkLock[myLine]->Release();
}//End Customer




//The Clerk thread
void Clerk(int whatLine){

	int myLine = whatLine;
	printf("Clerk %i: Initialized.\n", myLine);

	while(true){
		clerkLineLock->Acquire();
		if(clerkBribeLineCount[myLine] > 0){
			
			clerkBribeLineCV[myLine]->Signal(clerkLineLock);
			clerkState[myLine] = BUSY;
		}else if(false && clerkLineCount[myLine] > 0){
			printf("Clerk %i: Customer in line signalling...\n", myLine);
			clerkLineCV[myLine]->Signal(clerkLineLock);
			clerkState[myLine] = BUSY;
		}else{
			//eventually go on break //for now
			printf("Clerk %i: AVAILABLE.\n", myLine);
			clerkState[myLine] = AVAILABLE;
		}
		//For now release lock and continue...?
		clerkLineLock->Release(); //remove this after testing.
		continue;
		//Should only do this when we are BUSY?
		if(clerkState[myLine] == BUSY){
			clerkLock[myLine]->Acquire();
			clerkLineLock->Release();
			//wait for customer data
			clerkCV[myLine]->Wait(clerkLock[myLine]);
			//Do my job - customer waiting
			clerkCV[myLine]->Signal(clerkLock[myLine]);
			clerkCV[myLine]->Wait(clerkLock[myLine]);
			clerkLock[myLine]->Release();
		}
	}
}//End Clerk





//This runs the simulation
void Part2TestSuit(){

	for(int i = 0; i < CLERKCOUNT; i++){
		clerkLineCV.push_back(new Condition("clerkLineCV" + i));
		clerkBribeLineCV.push_back(new Condition("clerkLineCV" + i));
		clerkLock.push_back(new Lock("clerkLock" + i));
		clerkCV.push_back(new Condition("clerkCV" + i));
	}

	Thread *t;
	t = new Thread("Customer 0");
	t->Fork(Customer, 0);
	t = new Thread("Customer 1");
	t->Fork(Customer, 1);
	t = new Thread("Customer 2");
	t->Fork(Customer, 2);

	t = new Thread("Clerk 0");
	t->Fork(Clerk, 0);

}










#endif//PROJ1_PART2_H
