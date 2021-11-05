/* interrupts */

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"

extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProc;
extern pcb_t *readyQueue;
extern cpu_t start_clock;
extern int semD[SEMNUM];

/* Determines interrupt with the highest priority and passes control to the scheduler */
void interruptHandler(){

	cpu_t stop_clock;
	cpu_t time_left;
	time_left = getTimer();
	STCK(stop_clock);

	/* PLT interrupt, means it is time to switch to next process */
	if((((state_PTR)BIOSDATAPAGE)->s_cause & PLTINT) != 0){
		if(currentProc != NULL){
			/* get time of current process */
			currentProc->p_time = (currentProc->p_time) + (stop_clock - start_clock);
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
	if((((state_PTR)BIOSDATAPAGE)->s_cause & TIMERINT) != 0){
		pcb_PTR proc;
        LDIT(PSEUDOCLOCKTIME);
        proc = removeBlocked(&semD[SEMNUM-1]); /*CLOCKSEM = semD[SEMNUM-1]*/
        while(proc !=NULL){
            insertProcQ(&readyQue, proc);
            softBlockCount -= 1;
            proc = removeBlocked(&semD[SEMNUM-1]); /*CLOCKSEM = semD[SEMNUM-1]*/
        }
        /* set the semaphore to = 0 */
        semD[SEMNUM-1] = 0; /*CLOCKSEM = semD[SEMNUM-1]*/
        if(currentProc == NULL){
            scheduler();
        }
	}

	/* if it is not a PLT or pseudo clock interrupt and therefore a device interrupt */

	/* Disk interrupt */
    if((((state_PTR)BIOSDATAPAGE)->s_cause & DISKINT) != 0){
        /* disk dev is on */
        devInterruptH(DISK);
    }

    /* Flash interrupt */
    if((((state_PTR)BIOSDATAPAGE)->s_cause & FLASHINT) != 0){
        /* flash dev is on */
        devInterruptH(FLASH);
    }

    /* Printer interrupt */
    if((((state_PTR)BIOSDATAPAGE)->s_cause & PRINTERINT) != 0) {
        /* printer dev is on */
        devInterruptH(PRINTER);
    }

    /* Terminal interrupt (special case) */
    if((((state_PTR)BIOSDATAPAGE)->s_cause & TERMINT) != 0) {
        /* terminal dev is on */
        devInterruptH(TERMINAL);
    }
}

/* Interrupt Handler for peripheral devices */
HIDDEN int devInterruptH(int devLine){
	unsigned int bitMAP;
    volatile devregarea_t *deviceRegister;

    /* Addressing */
    deviceRegister = (devregarea_t *) RAMBASEADDR;
    bitMAP = deviceRegister->interrupt_dev[devLine-DISK];

    int device_number; /* interrupt device number */
    int device_semaphore; /* interrupt device semaphore */
    unsigned int intstatus; /* register status of the interrupting device */
    pcb_PTR p;

    /* logic to determine which specific device number is causing an interrupt */
    if((bitMAP & DEV0) != 0){
        device_number = 0;
    }else if((bitMAP & DEV1) != 0){
        device_number = 1;
    }else if((bitMAP & DEV2) != 0){
        device_number = 2;
    }else if((bitMAP & DEV3) != 0){
        device_number = 3;
    }else if((bitMAP & DEV4) != 0){
        device_number = 4;
    }else if((bitMAP & DEV5) != 0){
        device_number = 5;
    }else if((bitMAP & DEV6) != 0){
        device_number = 6;
    }else{
        device_number = 7;
    }

    /* get device semaphore */
    device_semaphore = ((devLine - DISK) * DEVPERINT) + device_number;

    /* For terminal interrupts */
    if(devLine == TERMINAL){
        intstatus = termInterruptH(&device_semaphore); /* call function for handling terminal interrupts */

    /* if not a terminal device interrupt */
    }else{
        intstatus = ((deviceRegister->devreg[device_semaphore]).d_status);
        /* ACK the interrupt */
        (deviceRegister->devreg[device_semaphore]).d_command = ACK;
    }

    /* V operation on the device semaphore */
    semD[device_semaphore] = semD[device_semaphore] + 1;

    /* if already waited for i/o */
    if(semD[device_semaphore] <= 0) {
        p = removeBlocked(&(semD[device_semaphore]));
        if (p != NULL) {
            p->p_s.s_v0 = intstatus; /* save status */
            insertProcQ(&readyQue, p); /* insert the process onto the ready queue */
            softBlockCount -= 1; /* update SBC*/
        }
    }
    /* if no process is running, call the scheduler to set the next process */
    if(currentProc == NULL){
        scheduler();
    }
}

/* Gives device status from terminal read or write case */
HIDDEN int termInterruptH(int *devSem){
	volatile devregarea_t *deviceRegister;
    unsigned int status;
    deviceRegister = (devregarea_t *) RAMBASEADDR;

    /* terminal write case takes priority over terminal read case */
    if ((deviceRegister->devreg[(*devSem)].t_transm_status & 0x0F) != READY) { /* handle write casse */
        status = deviceRegister->devreg[(*devSem)].t_transm_status;
        deviceRegister->devreg[(*devSem)].t_transm_command = ACK;

    }else{ /* handle read case if not write case */
        status = deviceRegister->devreg[(*devSem)].t_recv_status;
        deviceRegister->devreg[(*devSem)].t_recv_command = ACK;
        /* update dev sema4 for the terminal read situation */
        (*devSem) = (*devSem) + DEVPERINT;
    }
    return(status);
}

/* Go through all the registers in the old state and place them into new state */
void storeState(state_t *blocked, state_t *ready){
    for (int i = 0; i < STATEREGNUM; i++){
        ready->s_reg[i] = blocked->s_reg[i];
    }
    /*Move all of the contents from the old state into the new state*/
    ready->s_entryHI = blocked->s_entryHI;
    ready->s_status = blocked->s_status;
    ready->s_pc = blocked->s_pc;
    ready->s_cause = blocked->s_cause;

}