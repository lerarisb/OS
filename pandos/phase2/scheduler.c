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
extern cpu_t startClock;


/*use so you don't have to keep repeating this in other methods */
void contextSwitch(pcb_PTR p){
	debugE(1, 2, 3, 4);
	currentProc = p;
	LDST(&(p->p_s));
}

void timer(pcb_PTR currentProc, cpu_t time){
	STCK(startClock);
	setTIMER(time);
	contextSwitch(currentProc);
}

/* Uses the round robin algorithm for each process that will be executed */
void scheduler(){


	
	/* temp variable for the pcb that will be removed */
	pcb_t *temp;

	/* removes pcb from head of ready queue and stores the pointer in temp */
	temp = removeProcQ(&readyQueue);

	if (temp != NULL){
		timer(temp, QUANTUM);
	}

	if (processCount == 0){
		HALT();
	}
	else{

		if (processCount > 0){
			if(softBlockCount > 0){

				currentProc = NULL;
				/* need to set timer to a very large value */
				setTIMER(MAXTIME);

				unsigned int setStatus = (ALLOFF | IECON | IMON | TEBITON);
				setSTATUS(setStatus);
				WAIT();
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





