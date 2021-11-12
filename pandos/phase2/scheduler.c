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
extern pcb_t*readyQueue;


/*use so you don't have to keep repeating this in other methods */
void contextSwitch(pcb_PTR p){
	debugE(1, 2, 3, 4);
	currentProc = p;
	LDST(&(p->p_s));

	
	


}



/* Uses the round robin algorithm for each process that will be executed */
void scheduler(){


	
	/* temp variable for the pcb that will be removed */
	pcb_t *temp;

	/* removes pcb from head of ready queue and stores the pointer in temp */
	temp = removeProcQ(&readyQueue);

	if (temp != NULL){

	/*store pointer to pcb in current field */
	currentProc = temp;

	/*load PLT*/
	setTIMER(QUANTUM);

	debugC(1, 2, 3, 4);

	
	contextSwitch(currentProc);
	

	debugD(1, 2, 3,4);



	

}

/*if it goes to the else, it means that no process was stored on the ReadyQueue */
	else{

	/* If the process count is 0 then the job is completed so you can halt */
	if(processCount == 0){
		HALT();
	}

	/* if it makes it down here, we know the processCount > 0 */
	else{
		if(softBlockCount > 0){
			/* need to set timer to a very large value */
			setTIMER(MAXTIME);

			/* set status register to enable interrupts */
			currentProc->p_s.s_status = (ALLOFF | TEON | MASKON | IEON);

		}
		/* there is a deadlock */
		if(softBlockCount == 0){
			PANIC();
			}
	}
}

}

void debugC(int a, int b, int c, int d){
	a++;
	b++;
}

void debugD(int a, int b, int c, int d){
	a++;
	b++;
}

void debugE(int a, int b, int c, int d){
	a++;
	b++;
}





