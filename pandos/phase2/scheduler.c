/* scheduler */

/*written by Ben Leraris and Cole Shelgren */

/*The scheduler uses a round robin algorithm to determine the next process to be executed
It also contains helper methods
One helper method performs a context switch and gives control to a specificed process
The other method helps resent the timer */


#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "/usr/include/umps3/umps/libumps.h"

/* Variables used from initial.c */
extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProc;
extern pcb_t*readyQueue;
extern cpu_t startClock;

/*HELPER METHODS */

/*used to switch the context to the given process and then give it control by loading state */
void contextSwitch(pcb_PTR p){
	currentProc = p;
	LDST(&(p->p_s));
}
/*used to reset the timer of the given process to the specificed amount of time*/
void timer(pcb_PTR p, cpu_t time){
	STCK(startClock);
	setTIMER(time);
	contextSwitch(p);
}

/* Uses the round robin algorithm for each process that will be executed */
void scheduler(){

	/*checks to make sure there is a process on the readyQueue
	If there is, it it is set as the current process and the clock is started
	finally, control is given to that removed process */
	if(!emptyProcQ(readyQueue)){
		setTIMER(QUANTUM);
		currentProc = removeProcQ(&readyQueue);
		STCK(startClock);
		contextSwitch(currentProc);
	}

	/*if there is not a process on the readyQueue */
	else{
		
		currentProc = NULL;

		/*deadlock */
		if ((processCount > 0) && (softBlockCount == 0)){
				PANIC();
			}

		/*no more processes */
		else if (processCount ==0){
			HALT();
		}

		/*wait */
		else{
			setTIMER(MAXTIME);

			unsigned int setStatus = (ALLOFF | IECON | IMON | TEBITON);
			setSTATUS(setStatus);
			WAIT();
		}
	}
}








