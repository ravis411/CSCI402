// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}


////////////////////////////////////
// ock Impementation
///////////////////////////////////

Lock::Lock(char* debugName) {
    lockState = FREE;
    lockOwner = NULL;
    queue = new List;
}
Lock::~Lock() {
    delete queue;
}

//Attempts to acquire Lock
void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    
    if (isHeldByCurrentThread())
    {
        (void) interrupt->SetLevel(oldLevel);   // restore interrupts
        return;
    }

    if (lockState == FREE)
    {
        lockState = BUSY;
        lockOwner = currentThread;
    }else{
        //Put thread on this ock's wait Q and go to sleep
        queue->Append((void *)currentThread);
        currentThread->Sleep();
    }
    (void) interrupt->SetLevel(oldLevel);   // restore interrupts
}//End Acquire()


// Releases a lock and gives it to the next Thread in the Q
void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(!isHeldByCurrentThread()){
        //Only the current thread can release a lock
        //printf("%s: Only the lockOwner can release the lock\n", currentThread->Print()); //TODO is this the correct print message?
        (void) interrupt->SetLevel(oldLevel);   // restore interrupts
        return;
    }
    if(queue->IsEmpty()){//Q not empty
        lockOwner = (Thread *)queue->Remove();
        scheduler->ReadyToRun(lockOwner);
    }else{//Lock Q is empty
        lockState = FREE;
        lockOwner = NULL;
    }
    (void) interrupt->SetLevel(oldLevel);   // restore interrupts
}//End Release()


// true if the current thread
    // holds this lock.  Useful for
    // checking in Release, and in
    // Condition variable ops below.
bool Lock::isHeldByCurrentThread(){
    if(currentThread == lockOwner)
        return true;
    else
        return false;
}

/////////////////////////////////
//End Lock Impementation
/////////////////////////////////


// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Condition::Condition(char* debugName) { 
    waitingLock = NULL;
    queue = new List;
}

Condition::~Condition() {
    delete queue;
 }

void Condition::Wait(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(conditionLock == NULL){
        printf("%s: conditionLock NULL\n", currentThread->Print());
        (void) interrupt->SetLevel(oldLevel);   // restore interrupts
        return;
    }
    if (waitingLock == NULL) //no one waiting
    {
        waitingLock = conditionLock;
    }
    if(conditionLock != waitingLock){
        printf("%s: conditionLock != waitingLock\n", currentThread->Print());
        (void) interrupt->SetLevel(oldLevel);   // restore interrupts
        return;
    }
    //Ok to wait
    //Add thread to CV wait Q
    queue->Append((void *)currentThread);
    waitingLock->Release();
    currentThread->Sleep();
    waitingLock->Acquire();
    (void) interrupt->SetLevel(oldLevel);   // restore interrupts
}//End Wait()

void Condition::Signal(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(waitingLock == NULL){ //if no thread waiting
        (void) interrupt->SetLevel(oldLevel);   // restore interrupts
        return;
    }
    if(waitingLock != conditionLock){
        printf("%s: Signal conditionLock != waitingLock\n", currentThread->Print());
        (void) interrupt->SetLevel(oldLevel);   // restore interrupts
        return;
    }
    //Wakeup 1 waiting thread
     scheduler->ReadyToRun( (Thread *)queue->Remove() );
     if(queue->IsEmpty()){
        waitingLock == NULL;
     }
     (void) interrupt->SetLevel(oldLevel);   // restore interrupts
}

void Condition::Broadcast(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(waitingLock == NULL){ //no threads to wake //TODO: Check this please
        (void) interrupt->SetLevel(oldLevel);   // restore interrupts
        return;
    }
    if(conditionLock != waitingLock){
        printf("%s: Broadcast conditionLock != waitingLock\n", currentThread->Print());
    }

    (void) interrupt->SetLevel(oldLevel);   // restore interrupts
    while(!queue->IsEmpty()){
        Signal(conditionLock);
    }
}
