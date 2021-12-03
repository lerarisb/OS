/* interrupts */

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "/usr/include/umps3/umps/libumps.h"



extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProc;
extern pcb_t *readyQueue;
extern cpu_t startClock;
extern int devSemaphore[SEM4DEV];

HIDDEN void devInterruptH(int devLine);
HIDDEN int termInterruptH(int *devSem);

/* Determines interrupt with the highest priority and passes control to the scheduler */
void interruptHandler(){
	cpu_t stopClock;
	cpu_t timeLeft;

    STCK(stopClock);
	timeLeft = getTIMER();
	
    state_PTR interruptState = (state_PTR)BIOSDATAPAGE;

	/* PLT interrupt, means it is time to switch to next process */
	if(((interruptState->s_cause) & PLTINTERRUPT) != 0){
		if(currentProc != NULL){
			/* get time of current process */
			currentProc->p_time = (currentProc->p_time) + (stopClock - startClock);
			/* store processor state */
			storeState((state_PTR) BIOSDATAPAGE, &(currentProc->p_s));
			/* add proccess back to the readyQueue */
			insertProcQ(&readyQueue, currentProc);
			/* call scheduler to switch to next process */
			scheduler();
		}
		/* if there isn't a currentProc */
		else{
			PANIC();
		}
	}
	/* pseudo clock tick interrupt */
	if((interruptState->s_cause & TIMERINT) != 0){

		pcb_PTR proc;
        LDIT(PSEUDOCLOCKTIME);

        	proc = removeBlocked(&devSemaphore[SEM4DEV - 1]); 
       		while(proc != NULL){
            	insertProcQ(&readyQueue, proc);
           	 	cpu_t timeSpent = (proc->p_time) + (stopClock - startClock);
                proc->p_time = timeSpent;
                softBlockCount -= 1;
           		proc = removeBlocked(&devSemaphore[SEM4DEV - 1]); 
       		}
        /* set the semaphore to = 0 */
       		devSemaphore[SEM4DEV - 1] = 0; 
        	if(currentProc == NULL){
            	scheduler();
        	}
	}

	/* if it is not a PLT or pseudo clock interrupt and therefore a device interrupt */

	/* disk interrupt */
    if((interruptState->s_cause & DISKINTERRUPT) != 0){
        /* disk dev is on */
        devInterruptH(DISKINT);
    }
    /* flash interrupt */
    if((interruptState->s_cause & FLASHINTERRUPT) != 0){
        /* flash dev is on */
        devInterruptH(FLASHINT);
    }
    /* printer interrupt */
    if((interruptState->s_cause & PRINTERINTERRUPT) != 0) {
        /* printer dev is on */
        devInterruptH(PRNTINT);
    }
    /* terminal interrupt */
    if(((interruptState->s_cause) & TERMINTERRUPT) != 0) {
        /* terminal dev is on */
        devInterruptH(TERMINT);
    }

    if (currentProc!= NULL){
        currentProc->p_time = currentProc->p_time + (startClock - stopClock);
        storeState(interruptState, &(currentProc->p_s));
        timer(currentProc, timeLeft);
    }

    else{
        HALT();
    }

}

/* interrupt Handler for peripheral devices */
 HIDDEN void devInterruptH(int devLine){
    unsigned int bitMAP;
    volatile devregarea_t *deviceRegister;

    /* getting the dev address */
    deviceRegister = (devregarea_t *) RAMBASEADDR;
    bitMAP = deviceRegister->interrupt_dev[devLine-DISKINT];
    /* interrupt device number */
    int device_number; 
    /* interrupt device semaphore */
    int device_semaphore;
    /* register status of the interrupting device */
    unsigned int intStatus; 
    pcb_t *p;

    /* determine which device number is causing an interrupt */
    if((bitMAP & DEV0) != 0){
        device_number = 0;
    }
    else if((bitMAP & DEV1) != 0){
        device_number = 1;
    }
    else if((bitMAP & DEV2) != 0){
        device_number = 2;
    }
    else if((bitMAP & DEV3) != 0){
        device_number = 3;
    }
    else if((bitMAP & DEV4) != 0){
        device_number = 4;
    }
    else if((bitMAP & DEV5) != 0){
        device_number = 5;
    }
    else if((bitMAP & DEV6) != 0){
        device_number = 6;
    }
    else{
        device_number = 7;
    }
	
    /* get device semaphore */
    device_semaphore = ((devLine - DISKINT) * DEVPERINT) + device_number;
	
    /* terminal interrupts */
    if(devLine == TERMINT){
        intStatus = termInterruptH(&device_semaphore); /* call function for handling terminal interrupts */

    /* if not a terminal device interrupt */
    }
    else{
        intStatus = ((deviceRegister->devreg[device_semaphore]).d_status);
        /* ACK the interrupt */
        (deviceRegister->devreg[device_semaphore]).d_command = ACK;
    }

    /* V operation on the device semaphore */
    devSemaphore[device_semaphore]++;

    /* if already waited for i/o */
    if(devSemaphore[device_semaphore] <= 0) {
        p = removeBlocked(&(devSemaphore[device_semaphore]));
        if (p != NULL) {
	    /* save status */
            p->p_s.s_v0 = intStatus;
	    /* insert the process onto the ready queue */
            insertProcQ(&readyQueue, p);
	    /* update SBC*/
            softBlockCount -= 1; 
        }
    }
    /* if no process is running, call the scheduler */
    if(currentProc == NULL){
        scheduler();
    }
}

/* gives device status from terminal read or write case */
HIDDEN int termInterruptH(int *devSem){
	volatile devregarea_t *deviceRegister;
    unsigned int status;
    deviceRegister = (devregarea_t *) RAMBASEADDR;

    /* terminal write case takes priority over terminal read case */
    if ((deviceRegister->devreg[(*devSem)].t_transm_status & 0x0F) != READY) { /* handle write casse */
        status = deviceRegister->devreg[(*devSem)].t_transm_status;
        deviceRegister->devreg[(*devSem)].t_transm_command = ACK;

    }
    /* handle read case if not write case */
    else{ 
        status = deviceRegister->devreg[(*devSem)].t_recv_status;
        deviceRegister->devreg[(*devSem)].t_recv_command = ACK;
        /* update devSem for the terminal read */
        (*devSem) = (*devSem) + DEVPERINT;
    }
    return(status);
}

/* Go through all the registers in the old state and place them into new state */
void storeState(state_t *blocked, state_t *ready){
    int i;
    for (i = 0; i < STATEREGNUM; i++){
        ready->s_reg[i] = blocked->s_reg[i];
    }
    /* move contents from the old state into the new state*/
    ready->s_entryHI = blocked->s_entryHI;
    ready->s_status = blocked->s_status;
    ready->s_pc = blocked->s_pc;
    ready->s_cause = blocked->s_cause;
}

