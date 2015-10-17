// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256


class PageTableEntry: public TranslationEntry{
    public:
    bool stackPage; //True if this is a stack page...Should be deleted when this thread is removed..?
    int currentThreadID;    //The currentThreadID

    // Assignment operator does a deep copy
    PageTableEntry &operator=(const PageTableEntry& entry);
};


class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    Table fileTable;			// Table of openfiles

    void Fork(int nextInstruction);//Can be called to add a stack
    void Exit();//Deletes 8 pages of stack for current thread.

 private:
    PageTableEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
    unsigned int numNonStackPages;
					// address space
    int FindPPN();  //Returns a ppn if found otherwise prints a message and halts nachos
};

#endif // ADDRSPACE_H
