extern void test();
#include "../h/const.h"
#include "../h/types.h"
#include "../phase1/asl.c"
#include "../phase1/pcb.c"
#include "../scheduler.c"

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
	


	/*populate Processor 0 Pass Up Vector*/
	xxx->tlb_refillHandler = (memaddr) uTLB_RefillHandler

	/*set stack pointer ofr Nucleus TLB-Refill event 
	handler to top of Nucleus Stack Page*/

	/*set Nucleus exception handler address to address of 
	Level 3 Nucleus Function that is the entry point for exception 
	and interrupt handling*/

	xxx->exception_handler = (memaddr) foobar;

	/*set the stack pointer for Nucleus exception handler to top of
	nucleus stack page*/

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

	allocPcb();

	

	/*enable interrupts*/
	/*enable processor local timer*/
	/*kernel-mode on*/
	
	/*stack pointer set to RAMTOP*/
	/*pcb set to address of test*/
	p->p_s.s_sp = RAMTOP;
	p->p_s.s_pc = p->p_s.s_tp = (memaddr)test p->p_s.s_status;


	/*recursively set all process tree fields to NULL*/
	void setNull(pcb_ptr p){
		/*base case*/
		if (p->p_child = null){
			while (p->p_sib != null){
				p = NULL;
				p->p_sib

			}
		}
	}

	while (p->p_child != null){
			pcb_ptr newP = p->p_child;

	}

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