#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "../h/libumps.h"


extern void test();

/* nucleus initalization */

/* declare global variables */

int processCount;
pcb_ptr *currentProc;
pcb_ptr *readyQueue;
int softBlockCount;
int DevSemaphore[devINT * DEVPERINT];

#define clockSem;

/* pseudo clock semaphore */
	/* one semaphore for each eternal sub device */

int main(){
	
	/*set Clocksem to 0 */
	clockSem = 0;
	

	initPcbs();
	initAsl();
	

	passupvector_t passUp = (passupvector_t) *PASSUPVECTOR;
	/*populate Processor 0 Pass Up Vector*/
	
	passUp->tlb_refillHandler = (memaddr) uTLB_RefillHandler

	/*set stack pointer ofr Nucleus TLB-Refill event 
	handler to top of Nucleus Stack Page*/

	passUp->tlb_refll_stackPtr = KERNELSTACK;

	/*set Nucleus exception handler address to address of 
	Level 3 Nucleus Function that is the entry point for exception 
	and interrupt handling*/

	passUp->exception_handler = (memaddr) KERNELSTACK;

	/*set the stack pointer for Nucleus exception handler to top of
	nucleus stack page*/

	passUp->exception_stackPtr = (memaddr) PASSUPVECTOR;

	processCount = 0;
	softBlockCount = 0;
	readyQueue = mkEmptyProcQ();
	currentProc = NULL;

	/*set device semaphores to 0*/
	for (int i, i < DEVINT * DEVPERINT + 1){
		devSemaphore[i] = 0;
	}

	/*load system wide Interval timer with 100 ms */
	LDIT(100);

	p = allocPcb();
	insertProcQ(p);

	

	/*enable interrupts*/
	/*enable processor local timer*/
	/*kernel-mode on*/

	s_status = ALLOFF | TEON | MASKON | IEON
	
	/*stack pointer set to RAMTOP*/
	/*pcb set to address of test*/
	p->p_s.s_sp = RAMTOP;
	p->p_s.s_pc = p->p_s.s_tp = (memaddr)test p->p_s.s_status;


	/*accumulated time field to 0*/
	p_time = 0;

	/*set blocking semaphore address to NULL*/
	p_semAdd = NULL;

	/*set Support Structure Pointer pointer to null*/
	p_supportStruct = NULL;

	/*call the scheduler*/
	scheduler();

	return 0;

}

void uTLB_RefillHandler(){
	setENTRYHI( 0x80000000);
	setENTRYLO(0x00000000);
	TLBWR();
	LDST ((state_PTR) 0x0FFFF0000);
}

void genExceptionHandler(){
	pcb_ptr oldstate;
	int exceptReason;



	/*or bits together to determine cause*/
	oldState = BIOSDATAPAGE
	exceptReason = oldstate->s_cause & getExecCode>>causeShift
	syscall(exceptReason);

}