// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "synch.h"
#include "bitmap.h"
using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
		// Copy len bytes from the current thread's virtual address vaddr.
		// Return the number of bytes so read, or -1 if an error occors.
		// Errors can generally mean a bad virtual address was passed in.
		bool result;
		int n=0;			// The number of bytes copied in
		int *paddr = new int;

		while ( n >= 0 && n < len) {
			result = machine->ReadMem( vaddr, 1, paddr );
			while(!result) // FALL 09 CHANGES
		{
				result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
		}	
			
			buf[n++] = *paddr;
		 
			if ( !result ) {
	//translation failed
	return -1;
			}

			vaddr++;
		}

		delete paddr;
		return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
		// Copy len bytes to the current thread's virtual address vaddr.
		// Return the number of bytes so written, or -1 if an error
		// occors.  Errors can generally mean a bad virtual address was
		// passed in.
		bool result;
		int n=0;			// The number of bytes copied in

		while ( n >= 0 && n < len) {
			// Note that we check every byte's address
			result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

			if ( !result ) {
	//translation failed
	return -1;
			}

			vaddr++;
		}

		return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
		// Create the file with the name in the user buffer pointed to by
		// vaddr.  The file name is at most MAXFILENAME chars long.  No
		// way to return errors, though...
		char *buf = new char[len+1];	// Kernel buffer to put the name in

		if (!buf) return;

		if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
		}

		buf[len]='\0';

		fileSystem->Create(buf,0);
		delete[] buf;
		return;
}

int Open_Syscall(unsigned int vaddr, int len) {
		// Open the file with the name in the user buffer pointed to by
		// vaddr.  The file name is at most MAXFILENAME chars long.  If
		// the file is opened successfully, it is put in the address
		// space's file table and an id returned that can find the file
		// later.  If there are any errors, -1 is returned.
		char *buf = new char[len+1];	// Kernel buffer to put the name in
		OpenFile *f;			// The new open file
		int id;				// The openfile id

		if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
		}

		if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
		}

		buf[len]='\0';

		f = fileSystem->Open(buf);
		delete[] buf;

		if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
			delete f;
	return id;
		}
		else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
		// Write the buffer to the given disk file.  If ConsoleOutput is
		// the fileID, data goes to the synchronized console instead.  If
		// a Write arrives for the synchronized Console, and no such
		// console exists, create one. For disk files, the file is looked
		// up in the current address space's open file table and used as
		// the target of the write.
		
		char *buf;		// Kernel buffer for output
		OpenFile *f;	// Open file for output

		if ( id == ConsoleInput) return;
		
		if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
		} else {
				if ( copyin(vaddr,len,buf) == -1 ) {
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] buf;
			return;
	}
		}

		if ( id == ConsoleOutput) {
			for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
			}

		} else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
			f->Write(buf, len);
	} else {
			printf("%s","Bad OpenFileId passed to Write\n");
			len = -1;
	}
		}

		delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
		// Write the buffer to the given disk file.  If ConsoleOutput is
		// the fileID, data goes to the synchronized console instead.  If
		// a Write arrives for the synchronized Console, and no such
		// console exists, create one.    We reuse len as the number of bytes
		// read, which is an unnessecary savings of space.
		char *buf;		// Kernel buffer for input
		OpenFile *f;	// Open file for output

		if ( id == ConsoleOutput) return -1;
		
		if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
		}

		if ( id == ConsoleInput) {
			//Reading from the keyboard
			scanf("%s", buf);

			if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
			}
		} else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
			len = f->Read(buf, len);
			if ( len > 0 ) {
					//Read something from the file. Put into user's address space
						if ( copyout(vaddr, len, buf) == -1 ) {
				printf("%s","Bad pointer passed to Read: data not copied\n");
		}
			}
	} else {
			printf("%s","Bad OpenFileId passed to Read\n");
			len = -1;
	}
		}

		delete[] buf;
		return len;
}

void Close_Syscall(int fd) {
		// Close the file associated with id fd.  No error reporting.
		OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

		if ( f ) {
			delete f;
		} else {
			printf("%s","Tried to close an unopen file\n");
		}
}

////////////////////////////////////////////////////////
/*******************************************************
	Our implementations for proj2 above code was provided
	*****************************************************/

Lock kernel_threadLock("Kernel Thread Lock");
//int forkCalled = 0;

void kernel_thread(int vaddr){
	DEBUG('f', "IN kernel_thread.\n");
 	
	currentThread->space->Fork(vaddr);//add stack space to pagetable and init registers...

	//(ProcessTable->getProcessEntry(currentThread->space))->addThread();
	//kernel_threadLock.Acquire();
	//forkCalled--;
  //kernel_threadLock.Release();
	DEBUG('f', "End kernel_thread.\n");
	machine->Run();
	ASSERT(FALSE);
}

/* Fork a thread to run a procedure ("func") in the *same* address space 
 * as the current thread.
 */
void Fork_Syscall(int funct){
	kernel_threadLock.Acquire();
	(ProcessTable->getProcessEntry(currentThread->space))->addThread();
  kernel_threadLock.Release();
	Thread* t;
	DEBUG('f', "In fork syscall. funct = %i\n", funct);
	t = new Thread("Forked thread.");
	t->space = currentThread->space;
 // DEBUG('f', "CurrentSpace: %i  TSpace: %i\n", currentThread->space, t->space);
	t->Fork((VoidFunctionPtr)kernel_thread, funct); //kernel_thread??
	currentThread->Yield();//It should not be necessary to yield here
	DEBUG('f', "End of Fork Syscall.\n");
}//end Fork_Syscall


Lock execLock("ExecLock");
int execCalled = 0;

/************************************************************************
* Run the executable, stored in the Nachos file "name", and return the  *
* address space identifier                                              *
***********************************************************************/
//Lock* kernel_exec_lock = new Lock("Kernel Exec lock for filename...");
//char *kernel_execBUF = null;
void kernel_exec(int intName){
	char* name = (char*)intName;
	DEBUG('e', "Kernel_exec system call: FileName: %s \n\n", name);

	OpenFile *executable = fileSystem->Open(name);
	AddrSpace *space;

	if (executable == NULL) {
		printf("Unable to open file %s\n", name);
		return;
	}
	 
		space = new AddrSpace(executable);

		currentThread->space = space;
		//processTable.insert(space, (new ProcessTableEntry(space)));
		ProcessTable->addProcess(space);
		delete executable;      // close file

		space->InitRegisters();   // set the initial register values
		space->RestoreState();    // load page table register
		delete[] name;
		execLock.Acquire();
		execCalled--;
		execLock.Release();
		machine->Run();     // jump to the user progam
		ASSERT(FALSE);      // machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"*/
}

SpaceId Exec_Syscall(unsigned int vaddr, int len){
		execLock.Acquire();
		execCalled++;
		execLock.Release();
		DEBUG('e', "In exec syscall. vaddr: %i, len: %i\n", vaddr, len);

		char *buf;   // Kernel buffer
		
		if ( !(buf = new char[len]) ) {
			printf("%s","Error allocating kernel buffer for write!\n");
			return -1;
		} else {
				if ( copyin(vaddr,len,buf) == -1 ) {
					printf("%s","Bad pointer passed to to Exec: Exec aborted.\n");
					delete[] buf;
					return -1;
				}
		}

		//string name(buf);

		DEBUG('e' ,"The filename: %s\n", buf);
		//DEBUG('e' ,"Or as a string: %s\n", name.c_str());

		Thread* t;
		t = new Thread("Execed Thread.");
		t->Fork((VoidFunctionPtr)kernel_exec, (int)buf);

	return -1;
}



/*************************************************************************
* Exit()
*************************************************************************/
void Exit_Syscall(int status){
	ProcessTableEntry* p = ProcessTable->getProcessEntry(currentThread->space);
	execLock.Acquire();
	kernel_threadLock.Acquire();//TODO: RACE CONDITION THIS WONT WORK!!!?

	//Case 1
		//Not last thread in process
		//reclaim 8 pages of stack
		//vpn,ppn,valid
		//memoryBitMap->Clear(ppn)
		//valid = false
	if(p->getNumThreads() > 1){
		p->removeThread();
		DEBUG('E', "Not the last thread. Left: %i \n", p->getNumThreads());
		currentThread->space->Exit();
	}
	//Case 2
		//Last executing thread in last process
		//interupt->Halt();//shut downs nachos
	else if(p->getNumThreads() == 1 && ProcessTable->getNumProcesses() == 1 && !execCalled){
		DEBUG('E', "LAST THREAD LAST PROCESS\n");
		interrupt->Halt();
	}
	//Case 3
		//Last executing thread in a process - not last process
		//reclaim all unreclaimed memory
		//for(pageTable)
			//if valid clear
		//locks/cvs match addspace* w/ process table

	//Minumum this must have
	else{
		DEBUG('E', "Last thread in process.\n");
		ProcessTable->deleteProcess(currentThread->space);
		delete currentThread->space;
	}
	execLock.Release();
	kernel_threadLock.Release();

	currentThread->Finish();
}


/*************************************************************
//Prints and int
**************************************************************/
void PrintInt_Syscall(int wat){
	printf("%i", wat);
}

/*************************************************************
//Prints a String
**************************************************************/
void PrintString_Syscall(unsigned int vaddr, int len){
	char *buf;		// Kernel buffer for output
		
		
	if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return;
	} else {
		if ( copyin(vaddr,len,buf) == -1 ) {
			printf("%s","Bad pointer passed to to PrintString: data not pinted.\n");
			delete[] buf;
			return;
		}
	}

	for (int ii=0; ii<len; ii++) {
		printf("%c",buf[ii]);
	}

	delete[] buf;
}//End PrintStringSyscall




//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//	Lock Syscalls
///////////////////////////////////////////////////////////////////////////////////////////
class LockTableEntry{
public:
	Lock* lock;
	AddrSpace* space;
	bool isToBeDeleted;
};
#define lockTableSize 100
BitMap lockTableBitMap(lockTableSize);
LockTableEntry* lockTable[lockTableSize];


bool Lock_Syscall_InputValidation(int lock){
	if(lock < 0 || lock >= lockTableSize){
		printf("Invalid Lock Identifier: %i ", lock);
		return FALSE;
	}

	LockTableEntry* lockEntry = lockTable[lock];

	if(lockEntry == NULL){
		printf("Lock %i does not exist. ", lock);
		return FALSE;
	}
	if(lockEntry->space != currentThread->space){
		printf("Lock %i does not belong to this process. ", lock);
		return FALSE;
	}
	return TRUE;
}


///////////////////////////////
// Creates the Lock
///////////////////////////////
int CreateLock_Syscall(){
	DEBUG('L', "In CreateLock_Syscall\n");

	int lockTableIndex = lockTableBitMap.Find();
	if(lockTableIndex == -1){
		printf("Max Number of Locks created. Unable to CreateLock\n");
		return -1;
	}

	LockTableEntry* te = new LockTableEntry();
	te->lock = new Lock("Lock " + lockTableIndex);
	te->space = currentThread->space;
	te->isToBeDeleted = FALSE;

	lockTable[lockTableIndex] = te;

	return lockTableIndex;
}


/***********************
*	Acquire the lock
*/
void Acquire_Syscall(int lock){
	DEBUG('L', "In Acquire_Syscall\n");

	if(!Lock_Syscall_InputValidation(lock)){
	 printf("Unable to Acquire.\n");
	 return;
	}

	DEBUG('L', "Acquiring lock.\n");
	lockTable[lock]->lock->Acquire();

}

/*****************
* 	Release the lock
*/
void Release_Syscall(int lock){
	DEBUG('L', "In Release_Syscall\n");

	if(!Lock_Syscall_InputValidation(lock)){
	 printf("Unable to Release.\n");
	 return;
	}
	
	LockTableEntry* le = lockTable[lock];

	DEBUG('L', "Releasing lock.\n");
	le->lock->Release();
	
	if(le->isToBeDeleted && !(le->lock->isBusy()) ){
		DEBUG('L', "Lock %i no longer busy. Deleting.\n", lock);
		delete le->lock;
		le->lock = NULL;
		delete le;
		lockTable[lock] = NULL;
		lockTableBitMap.Clear(lock);
	}

}

void DestroyLock_Syscall(int lock){
	DEBUG('L', "In DestroyLock_Syscall\n");

	if(!Lock_Syscall_InputValidation(lock)){
	 printf("Unable to DestroyLock.\n");
	 return;
	}

	LockTableEntry* le = lockTable[lock];

	if((le->lock->isBusy()) ){
		le->isToBeDeleted = TRUE;
		DEBUG('L', "Lock %i BUSY marking for deletion.\n", lock);
	}else{
		delete le->lock;
		le->lock = NULL;
		delete le;
		lockTable[lock] = NULL;
		lockTableBitMap.Clear(lock);
		DEBUG('L', "Lock %i deleted.\n", lock);
	}


}



//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//	Condition Syscalls
///////////////////////////////////////////////////////////////////////////////////////////

class ConditionTableEntry{
public:
	Condition* condition;
	AddrSpace* space;
	bool isToBeDeleted;
};

#define ConditionTableSize 100
BitMap ConditionTableBitMap(ConditionTableSize);
ConditionTableEntry* ConditionTable[ConditionTableSize];


bool Condition_Syscall_InputValidation(int cond, int lock){

	if(!Lock_Syscall_InputValidation(lock)){
	 return FALSE;
	}

	if(cond < 0 || cond >= ConditionTableSize){
		printf("Invalid Condition Identifier: %i ", cond);
		return FALSE;
	}

	ConditionTableEntry* condEntry = ConditionTable[cond];
	if(condEntry == NULL){
		printf("Condition %i does not exist. ", cond);
		return FALSE;
	}
	if(condEntry->space != currentThread->space){
		printf("Lock %i does not belong to this process. ", cond);
		return FALSE;
	}
	return TRUE;
}



int CreateCondition_Syscall(){
	DEBUG('C', "In CreateCondition_Syscall\n");
	
	int conID = ConditionTableBitMap.Find();
	if(conID == -1){
		printf("Max Number of Conditions created. Unable to CreateCondition\n");
		return -1;
	}

	ConditionTableEntry* ce = new ConditionTableEntry();

	ce->condition = new Condition("Condition " + conID);
	ce->space = currentThread->space;
	ce->isToBeDeleted = FALSE;

	ConditionTable[conID]	= ce;

	return conID;
}

void Wait_Syscall(int condition, int lock){
	DEBUG('C', "In Wait_Syscall\n");

	if(!Condition_Syscall_InputValidation(condition, lock)){
		printf("Unable to Wait.\n");
		return;
	}

	ConditionTableEntry* ce = ConditionTable[condition];
	LockTableEntry* le = lockTable[lock];

	ce->condition->Wait(le->lock);
}

void Signal_Syscall(int condition, int lock){
	DEBUG('C', "In Signal_Syscall\n");

	if(!Condition_Syscall_InputValidation(condition, lock)){
		printf("Unable to Signal.\n");
		return;
	}

	ConditionTableEntry* ce = ConditionTable[condition];
	LockTableEntry* le = lockTable[lock];

	ce->condition->Signal(le->lock);

	if(ce->isToBeDeleted && !ce->condition->isBusy()){
		DEBUG('C', "Condition %i no longer BUSY. Deleting.");
		ConditionTable[condition] = NULL;
		delete ce->condition;
		ce->condition = NULL;
		delete ce;
		ConditionTableBitMap.Clear(condition);
	}

}

void Broadcast_Syscall(int condition, int lock){
	DEBUG('C', "In Broadcast_Syscall\n");

	if(!Condition_Syscall_InputValidation(condition, lock)){
		printf("Unable to Broadcast.\n");
		return;
	}

	ConditionTableEntry* ce = ConditionTable[condition];
	LockTableEntry* le = lockTable[lock];

	ce->condition->Broadcast(le->lock);


	if(ce->isToBeDeleted && !ce->condition->isBusy()){
		DEBUG('C', "Condition %i no longer BUSY. Deleting.");
		ConditionTable[condition] = NULL;
		delete ce->condition;
		ce->condition = NULL;
		delete ce;
		ConditionTableBitMap.Clear(condition);
	}
}

void DestroyCondition_Syscall(int condition){
	DEBUG('C', "In DestroyCondition_Syscall\n");

	ConditionTableEntry* ce = ConditionTable[condition];

	if((ce->condition->isBusy()) ){
		ce->isToBeDeleted = TRUE;
		DEBUG('C', "Condition %i BUSY marking for deletion.\n", condition);
	}else{
		ConditionTable[condition] = NULL;
		delete ce->condition;
		ce->condition = NULL;
		delete ce;
		ConditionTableBitMap.Clear(condition);
		DEBUG('C', "Condition %i deleted.\n", condition);
	}

}











void ExceptionHandler(ExceptionType which) {
		int type = machine->ReadRegister(2); // Which syscall?
		int rv=0; 	// the return value from a syscall

		if ( which == SyscallException ) {
	switch (type) {
			default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
			case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
			case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
			case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
			case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
						machine->ReadRegister(5),
						machine->ReadRegister(6));
		break;
			case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
						machine->ReadRegister(5),
						machine->ReadRegister(6));
		break;
			case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
			
		case SC_Fork:
			DEBUG('a', "Fork syscall.\n");
			Fork_Syscall(machine->ReadRegister(4));
		break;

		case SC_Yield:
			DEBUG('a', "Yield syscall.\n");
			currentThread->Yield();
		break;

		case SC_PrintInt:
			DEBUG('a', "PrintInt syscall.\n");
			PrintInt_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_PrintString:
			DEBUG('a', "PrintString syscall.\n");
			PrintString_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
		
		case SC_Exit:
			DEBUG('a', "Exit syscall.\n");
			Exit_Syscall(machine->ReadRegister(4));
		break;

		case SC_Exec:
			DEBUG('a', "Exec syscall.\n");
			rv = Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;

		case SC_CreateLock:
			DEBUG('a', "CreateLock syscall.\n");
			rv = CreateLock_Syscall();
		break;

		case SC_Acquire:
			DEBUG('a', "Acquire syscall.\n");
			Acquire_Syscall(machine->ReadRegister(4));
		break;

		case SC_Release:
			DEBUG('a', "Release syscall.\n");
			Release_Syscall(machine->ReadRegister(4));
		break;

		case SC_DestroyLock:
			DEBUG('a', "DestroyLock syscall.\n");
			DestroyLock_Syscall(machine->ReadRegister(4));
		break;

		case SC_CreateCondition:
			DEBUG('a', "CreateCondition syscall.\n");
			rv = CreateCondition_Syscall();
		break;

		case SC_Wait:
			DEBUG('a', "Wait syscall.\n");
			Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;

		case SC_Signal:
			DEBUG('a', "Signal syscall.\n");
			Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;

		case SC_Broadcast:
			DEBUG('a', "Broadcast syscall.\n");
			Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;

		case SC_DestroyCondition:
			DEBUG('a', "DestroyCondition syscall.\n");
			DestroyCondition_Syscall(machine->ReadRegister(4));
		break;

	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
		} else {
			cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
			interrupt->Halt();
		}
}
