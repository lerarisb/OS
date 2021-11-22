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

/* nucleus initalization */

/* declare global variables */

int processCount;
pcb_t *currentProc;
pcb_t *readyQueue;
int softBlockCount;
int devSemaphore[SEM4DEV];
cpu_t startClock;

#define clockSem devSemaphore[SEM4DEV - 1]

/* pseudo clock semaphore */
	/* one semaphore for each eternal sub device */

int main(){
	
	

	initPcbs();
	initASL();
	
	

	passupvector_t *passUp = (passupvector_t*) PASSUPVECTOR;
	/*populate Processor 0 Pass Up Vector*/
	
	passUp->tlb_refll_handler = (memaddr) uTLB_RefillHandler;

	/*set stack pointer ofr Nucleus TLB-Refill event 
	handler to top of Nucleus Stack Page*/

	passUp->tlb_refll_stackPtr = KERNELSTACK;

	/*set Nucleus exception handler address to address of 
	Level 3 Nucleus Function that is the entry point for exception 
	and interrupt handling*/

	passUp->exception_handler = (memaddr) genExceptionHandler;

	/*set the stack pointer for Nucleus exception handler to top of
	nucleus stack page*/

	passUp->exception_stackPtr = (memaddr) KERNELSTACK;




	processCount = 0;
	softBlockCount = 0;
	readyQueue = mkEmptyProcQ();
	currentProc = NULL;
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

	p = allocPcb();
	
	if (p != NULL){
		/*enable interrupts*/
		/*enable processor local timer*/
		/*kernel-mode on*/


		memaddr ramtop = *(int*)RAMBASEADDR + *(int*)RAMBASESIZE;

		p->p_s.s_status = (ALLOFF | IECON | IMON | TEBITON);
	
		/*stack pointer set to RAMTOP*/
		/*pcb set to address of test*/
		p->p_s.s_sp = (memaddr) ramtop;
		p->p_s.s_pc = (memaddr) test; 
		p->p_s.s_t9 = (memaddr) test;

		/*set Support Structure Pointer pointer to null*/
		p->p_supportStruct = NULL;

		
		processCount++;
		insertProcQ(&readyQueue, p);



		/*accumulated time field to 0*/
		/* p->p_time = 0; */

		/*set blocking semaphore address to NULL*/
		/* p->p_semAdd = NULL; */

	

		debugB(1, 2, 3, 4);

		/*call the scheduler*/
		scheduler();

	

	
	}
	else{
		PANIC();
	}
	return 0;
}

void genExceptionHandler(){
	
	debugExceptionHandlerBeforeOr(1, 2, 3, 4);

	state_PTR oldState;
	int exceptReason;

	

	/*or bits together to determine cause*/
	oldState = (state_PTR) BIOSDATAPAGE;
	exceptReason = (oldState->s_cause & GETEXECCODE) >> CAUSESHIFT;

	debugCause(exceptReason);

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

void debugB(int a, int b, int c, int d){
	a = a + 2;
	b = b + 2;
}

void debugExceptionHandlerBeforeOr(int a, int b, int c, int d){
	a = a + 2;
	b = b + 2;
}

void debugExceptionHandlerAfterOr(int a, int b, int c, int d){
	a = a + 2;
	b = b + 2;
}

void debugCause(int i){
	i++;
}