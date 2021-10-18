extern void test();
#include "../h/const.h"
#include "../h/types.h"
#include "../phase1/asl.c"
#include "../phase1/pcb.c"
nclude "../scheduler.c"

/* nucleus initalization */

/* declare global variables */

int processCount;
pcb_ptr *currentProc;
pcb_ptr *readyQueue;
int softBlockCount;
int DevSemaphore[?];
/* pseudo clock semaphore */
	/* one semaphore for each eternal sub device */

void main(){
	initPcbs();
	initAsl();
	

	/*populate Processor 0 Pass Up Vector*/
	xxx->tlb_refillHandler = (memaddr) uTLB_RefillHandler

	/*set stack pointer ofr Nucleus TLB-Refill event 
	handler to top of Nucleus Stack Page*/

	/*set Nucleus exception handler address to address of 
	Level 3 Nucleus Function that is the entry point for exception 
	and interrupt handling*/

	xxx->exception_handler = (memaddr) foobar;

	/*set the stack pointer for Nucleus exceptio handler to top of
	nucleus stack page*/

	processCount = 0;
	softBlockCount = 0;
	readyQueue = mkEmptyProcQ();
	currentProc = NULL;
	/*set device semaphores to 0*/

	/*load system wide Unterval timer with 100 ms */

	allocPcb();
	/*enable interrupts*/
	/*enable processor local timer*/
	/*kernel-mode on*/
	/*stack pointer set to RAMTOP*/
	/*pcb set to address of test*/

	/*set all process tree fields to NULL*/

	/*accumulated time field to 0*/
	p_time = 0;

	/*set blocking semaphore address to NULL*/
	p_semAdd = NULL;

	/*set Support Structure Pointer pointer to null*/
	p_supportStruct = NULL;

	/*call the scheduler*/
	scheduler();

}

void uTLB_RefillHandler(){
	setENTRYHI( 0x80000000);
	setENTRYLO(0x00000000);
	TLBWR();
	LDST ((state_PTR) 0x0FFFF0000);
}
