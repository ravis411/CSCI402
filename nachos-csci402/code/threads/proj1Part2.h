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
enum CLERKSTATE = {AVAILABLE, BUSY, ONBREAK}		//enum for the CLERKSTATE


//Initialize Locks, CVS, and Monitors?
Lock clerkLineLock("clerkLineLock");						//The clerkLineLock
std::vector<int> clerkLineCount(CLERKCOUNT, 0);				//clerkLineCount
std::vector<CLERKSTATE> clerkState(CLERKCOUNT, AVAILABLE);	//clerkState
std::vector<Condition> clerkLineCV;
std::vector<Lock> clerkLock;
std::vector<Condition> clerkCV;
for(int i = 0; i < CLERKCOUNT; i++){
	clerkLineCV.push_back(new Condition("clerkLineCV" + i));
	clerkLock.push_back(new Lock("clerkLock" + i));
	clerkCV.push_back(new Condition("clerkCV" + i));
}


//The Customer thread
void Customer(){
	clerkLineLock->Acquire();
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
		clerkLineCV[myLine]->wait(clerkLineLock);
		clerkLineCount[myLine]--;
	}
	//Clerk is AVAILABLE
	clerkState[myLine] = BUSY;
	clerkLineLock->Release();
	printf("Customer %s: Done Returning.\n", myLine);
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

	myLine = whatLine;
	printf("Clerk %s: Initialized.\n", myLine);
	while(true){
		clerkLineLock->Acquire();
		if(clerkBribeLineCount[myLine] > 0){
			
			clerkBribeLineCV[myLine]->Signal(clerkLineLock);
			clerkState[myLine] = BUSY;
		}else if(false && clerkLineCount[myLine] > 0){
			printf("Clerk %s: Customer in line signalling...\n", myLine);
			clerkLineCV[myLine]->Signal(clerkLineLock);
			clerkState[myLine] = BUSY;
		}else{
			//eventually go on break //for now
			printf("Clerk %s: AVAILABLE.\n", myLine);
			clerkState[myLine] = AVAILABLE;
		}
		//For now continue...?
		continue;
		//Should only do this when we are BUSY?
		if(clerkState[myLine] == BUSY){
			clerkLock[myLine]->Acquire();
			clerkLineLock->Release();	//TODO: is this right?
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

	Thread *t;
	for()

}










#endif//PROJ1_PART2_H