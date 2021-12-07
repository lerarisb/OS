/*nucleus initalization */

/*written by Ben Leraris and Cole Shelgren */

/*this is the entry point for pandos that performs nucleus initalizaiton */
/*this file contains and initalizes the following global variables
	an integer for the number of started but not terminated processes
	an integer for the number of blocked processes
	tail pointer to a ready queue containing pcbs that are ready
	pointer to the pcb that is currently executing
	semaphores for each device stored in an array
	the value the time of day clock started at 
	the semaphore for the pseudo clock
this file also populates the pass up vector
it initalizes pcb and asl data structure by calling initPcb and initSemd
laods the system wide interval timer with 100 ms
instantiates a single process, places it in the readyQueue, and increments process count
finally, calls the scheduler

*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "/usr/include/umps3/umps/libumps.h"


extern void test();


extern void uTLB_RefillHandler();
/* nucleus initalization */

/* declare global variables */
/*
an integer for the number of started but not terminated processes*/
int processCount;

/*pointer to the pcb that is currently executing*/
pcb_t *currentProc;

/*tail pointer to a ready queue containing pcbs that are ready*/
pcb_t *readyQueue;

/*an integer for the number of blocked processes*/
int softBlockCount;

/*semaphroes for each device stored in an array*/
int devSemaphore[SEM4DEV];

/*the value the time of day clock started at */
cpu_t startClock;

/*he semaphore for the pseudo clock*/
#define clockSem devSemaphore[SEM4DEV - 1]

void main(){
	
	
	/* initalize pcbs and asl */
	initPcbs();
	initASL();
	
	
	/*populate Processor 0 Pass Up Vector*/
	passupvector_t *passUp = (passupvector_t*) PASSUPVECTOR;
	
	/*set address of the refull handler */
	passUp->tlb_refll_handler = (memaddr) uTLB_RefillHandler;

	/*set stack pointer ofr Nucleus TLB-Refill event 
	handler to top of Nucleus Stack Page*/

	passUp->tlb_refll_stackPtr = (memaddr) KERNELSTACK;

	/*set Nucleus exception handler address to address of 
	Level 3 Nucleus Function that is the entry point for exception 
	and interrupt handling*/

	passUp->exception_handler = (memaddr) genExceptionHandler;

	/*set the stack pointer for Nucleus exception handler to top of
	nucleus stack page*/

	passUp->exception_stackPtr = (memaddr) KERNELSTACK;


	/*no processes running */
	processCount = 0;

	/*no processes blocked */
	softBlockCount = 0;

	/*no processes on readyQueue */
	readyQueue = mkEmptyProcQ();

	/*no currently running process */
	currentProc = NULL;

	/*local variable */
	pcb_t *p;

	/*set device semaphores to 0*/
	int i;
	for (i=0; i < (SEM4DEV-1); i++){
		devSemaphore[i] = 0;
	}

	/*set Clocksem to 0 */
	clockSem = 0;
	

	/*load system wide Interval timer with 100 ms */
	LDIT(PSEUDOCLOCKTIME);

	/*create speace for currently running process */
	p = allocPcb();
	
	if (p != NULL){
		/*enable interrupts*/
		/*enable processor local timer*/
		/*kernel-mode on*/


		memaddr ramtop = *(int*)RAMBASEADDR + *(int*)RAMBASESIZE;


		/*stack pointer set to RAMTOP*/
		/*pcb set to address of test*/
		p->p_s.s_sp = (memaddr) ramtop;
		p->p_s.s_pc = (memaddr) test; 
		p->p_s.s_t9 = (memaddr) test;

		/*set Support Structure Pointer pointer to null*/
		p->p_supportStruct = NULL;
		
		p->p_s.s_status = (ALLOFF | IECON | IMON | TEBITON);
	

		/*increement process count*/
		processCount++;

		/*add to ready Queue */
		insertProcQ(&readyQueue, p);

		/*call the scheduler*/
		scheduler();
	}
	else{
		PANIC();
	}
}

/*responsible for determining cause of exception and sending to according file */

void genExceptionHandler(){
	
	/*local variable for old state */
	state_PTR oldState;

	/*local variable for except reason */
	int exceptReason;

	/*or bits together to determine cause*/
	oldState = (state_PTR) BIOSDATAPAGE;
	exceptReason = (oldState->s_cause & GETEXECCODE) >> CAUSESHIFT;

	if (exceptReason == INTERRUPT){
		interruptHandler();
	}

	if (exceptReason <= TLBCAUSE){
		TLBHandler();
	}

	if (exceptReason == SYSCALLHANDLE){
		sysHandler();
	}

	else{
		ProgramTrapHandler();
	}

}

