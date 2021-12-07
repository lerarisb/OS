/* interrupts */

/*written by Ben Leraris and Cole Shelgren */

/*interrupt occurs when a prveiously initated I/O request completes or when a timer makes an according transition
there are two types of interrupts - timer and devices
there are two types of timer interrupts - pseudo clock interrupts and processor local timer ones
this file contains an interrupt handler which determines what type the interryupt is, which interrupt lines have pending interrupts and which device per interrupt line
the interrupt is occuring on
terminal devices are treated as two subdevices
when there are multiple interrupts, the interrupt on the lowest line is given highest priority
there are methods for when the interrupt is a pseudo clock, processor local timer, non-terminal device, or device interrupts
there is also a helper method called store state that goes through all the registers in the old state and place them into new state  */

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
HIDDEN int findDeviceNum(int lineNum);

/* Determines interrupt with the highest priority and passes control to the scheduler */
void interruptHandler(){
	
    /*stop clock and get value */
    cpu_t stopClock;
    STCK(stopClock);

    /*local variable */
	cpu_t timeLeft;
	timeLeft = getTIMER();
	
    /*interrupt state */
    state_PTR interruptState = (state_PTR)BIOSDATAPAGE;

	/* PLT interrupt, means it is time to switch to next process */
	if(((interruptState->s_cause) & PLTINTERRUPT) != 0){
		Pltinterrupt();
	}

	/* pseudo clock tick interrupt */
	if((interruptState->s_cause & TIMERINT) != 0){
        PctInterrupt();
    }

	/* if it is not a PLT or pseudo clock interrupt and therefore a device interrupt */

	/* disk interrupt */
    if((interruptState->s_cause & DISKINTERRUPT) != 0){
        devInterruptH(DISKINT);
    }
    /* flash interrupt */
    if((interruptState->s_cause & FLASHINTERRUPT) != 0){
        devInterruptH(FLASHINT);
    }
    /* printer interrupt */
    if((interruptState->s_cause & PRINTERINTERRUPT) != 0) {
        devInterruptH(PRNTINT);
    }
    /* terminal interrupt */
    if(((interruptState->s_cause) & TERMINTERRUPT) != 0) {
        devInterruptH(TERMINT);
    }

/*update state and time of currentProc if not null */
    if (currentProc!= NULL){
        currentProc->p_time = currentProc->p_time + (startClock - stopClock);
        storeState(interruptState, &(currentProc->p_s));
        timer(currentProc, timeLeft);
        contextSwitch(currentProc);
    }

/*error*/
    else{
        HALT();
    }


}

/* interrupt Handler for peripheral devices */
 HIDDEN void devInterruptH(int lineNum){
    /* gettind device address */
    unsigned int bitMAP;
    volatile devregarea_t *deviceRegister;
    deviceRegister = (devregarea_t *) RAMBASEADDR;
    bitMAP = deviceRegister->interrupt_dev[lineNum-DISKINT];
    /* interrupt device number */
    int deviceNumber; 
    /* interrupt device semaphore */
    int deviceSemaphore;
    /* register status for the interrupting device */
    unsigned int intStatus; 
    pcb_t *p;

    /* determine which device number is causing an interrupt */
    if((bitMAP & DEV0) != 0){
        deviceNumber = 0;
    }
    else if((bitMAP & DEV1) != 0){
        deviceNumber = 1;
    }
    else if((bitMAP & DEV2) != 0){
        deviceNumber = 2;
    }
    else if((bitMAP & DEV3) != 0){
        deviceNumber = 3;
    }
    else if((bitMAP & DEV4) != 0){
        deviceNumber = 4;
    }
    else if((bitMAP & DEV5) != 0){
        deviceNumber = 5;
    }
    else if((bitMAP & DEV6) != 0){
        deviceNumber = 6;
    }
    else{
        deviceNumber = 7;
    }
	
    /* get the devices semaphore */
    deviceSemaphore = ((lineNum - DISKINT) * DEVPERINT) + deviceNumber;
	
    /* terminal interrupts */
    if(lineNum == TERMINT){
        intStatus = termInterruptH(&deviceSemaphore); 

    }
    /* if it is not a terminal device interrupt */
    else{
        intStatus = ((deviceRegister->devreg[deviceSemaphore]).d_status);
        /* ACK the interrupt */
        (deviceRegister->devreg[deviceSemaphore]).d_command = ACK;
    }

    /* V operation on the device semaphore */
    devSemaphore[deviceSemaphore]++;

    /* check if it is already waiting for i/o */
    if(devSemaphore[deviceSemaphore] <= 0) {
        p = removeBlocked(&(devSemaphore[deviceSemaphore]));
        if (p != NULL) {
	    /* save status */
            p->p_s.s_v0 = intStatus;
	    /* insert the process onto the ready queue */
            insertProcQ(&readyQueue, p);
	    /* update SBC */
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

    /* write case takes priority over read case */
    if ((deviceRegister->devreg[(*devSem)].t_transm_status & 0x0F) != READY) { /* handle write casse */
        status = deviceRegister->devreg[(*devSem)].t_transm_status;
        deviceRegister->devreg[(*devSem)].t_transm_command = ACK;

    }
    /* read case */
    else{ 
        status = deviceRegister->devreg[(*devSem)].t_recv_status;
        deviceRegister->devreg[(*devSem)].t_recv_command = ACK;
        /* update devSem for the terminal read */
        (*devSem) = (*devSem) + DEVPERINT;
    }
    return(status);
    

}

/*helper method */
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

/*use for PLT interrupt */
void Pltinterrupt(){
    cpu_t stopClock;
    STCK(stopClock);
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

/*used for pseudo clock timer interrupt */
void PctInterrupt(){
    cpu_t stopClock;
    STCK(stopClock);
    pcb_PTR proc;

        /*load time to max time */
        LDIT(PSEUDOCLOCKTIME);

        /*wake up any processes that went to sleep on this semaphore */
            proc = removeBlocked(&devSemaphore[SEM4DEV - 1]); 
            while(proc != NULL){
                insertProcQ(&readyQueue, proc);

                /*update the time charged to that process */
                cpu_t timeSpent = (proc->p_time) + (stopClock - startClock);
                proc->p_time = timeSpent;

                /*update soft block count */
                softBlockCount -= 1;
                proc = removeBlocked(&devSemaphore[SEM4DEV - 1]); 
            }
        /* set the semaphore to = 0 */
            devSemaphore[SEM4DEV - 1] = 0; 
            if(currentProc == NULL){
                scheduler();
            }
    }



