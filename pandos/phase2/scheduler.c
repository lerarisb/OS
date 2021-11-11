/* scheduler */


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
extern pcb_t *readyQueue;


/*use so you don't have to keep repeating this in other methods */
contextSwitch(pcb_PTR p){
	currentProc = p;
	LDST(&(p->p_s));

}



/* Uses the round robin algorithm for each process that will be executed */
void scheduler(){
	/* temp variable for the pcb that will be removed */
	pcb_t *temp;

	/* removes pcb from head of ready queue and stores the pointer in temp */
	temp = removeProcQ(&readyQueue);

	/* If the process count is 0 then the job is completed so you can halt */
	if(processCount == 0){
		HALT();
	}

	else if (processCount > 0){
		if(softBlockCount > 0){
			/* need to set timer still */

			/* enable all interrupts I believe */
			/* not sure if this is used to enable the interrupts or PLT */
			unsigned int sendStatus = ALLOFF | IECON | IMON | TEBITON;

			currentProc = currentProc->p_next;
			LDST(&(temp->p_s));
			WAIT();
		}
		/* there is a deadlock */
		if(softBlockCount == 0){
			PANIC();
		}
	}
}
