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

/*determines the cause of interrupt and passes to appropraite method */
void interruptHandler(){
	
    /*stop clock and get value */
    cpu_t stopClock;
    STCK(stopClock);

    /*local variable */
	cpu_t timeLeft;
	timeLeft = getTIMER();
	
    /*interrupt state */
    state_PTR interruptState = (state_PTR)BIOSDATAPAGE;

	/* cause was a PLT interrupt*/
	if(((interruptState->s_cause) & PLTINTERRUPT) != 0){
		Pltinterrupt();
	}

	/* cause was a pseudo clock tick interrupt */
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

/* method for device interrupts that determines device numbers
and performs v on appropriate device semaphore */
 HIDDEN void devInterruptH(int lineNum){
    /* gettind device address */
    unsigned int bitMAP;
    volatile devregarea_t *deviceRegister;
    deviceRegister = (devregarea_t *) RAMBASEADDR;
    bitMAP = deviceRegister->interrupt_dev[lineNum-DISKINT];
    /* local variable interrupt device number */
    int deviceNumber; 
    /* local variable interrupt device semaphore */
    int deviceSemaphore;
    /* local variable register status for the interrupting device */
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
	
    /* use for terminal interrupts because terminal really has two sub devices */
    if(lineNum == TERMINT){
        intStatus = termInterruptH(&deviceSemaphore); 

    }
    /* if it is not a terminal device interrupt */
    else{
        intStatus = ((deviceRegister->devreg[deviceSemaphore]).d_status);
        /* ACK the interrupt */
        (deviceRegister->devreg[deviceSemaphore]).d_command = ACK;
    }

    /* perform V on device semaphore */
    devSemaphore[deviceSemaphore]++;

    /* check if it is already waiting for i/o */
    if(devSemaphore[deviceSemaphore] <= 0) {
        p = removeBlocked(&(devSemaphore[deviceSemaphore]));
        if (p != NULL) {
	    /* saves the register status */
            p->p_s.s_v0 = intStatus;
	    /* inserts the process onto the readyQueue */
            insertProcQ(&readyQueue, p);
	    /* updates the softBlockCount */
            softBlockCount -= 1; 
        }
    }
    /* if no process is currently running, call the scheduler to choose next one */
    if(currentProc == NULL){
        scheduler();
    }


}

/* used when terminal interrupt  to update status and acknowledge interrupt
terminal can be in write or in read */
HIDDEN int termInterruptH(int *devSem){
	volatile devregarea_t *deviceRegister;
    unsigned int status;
    deviceRegister = (devregarea_t *) RAMBASEADDR;

    /* if terminal is in write */
    if ((deviceRegister->devreg[(*devSem)].t_transm_status & 0x0F) != READY) { /* handle write casse */
        status = deviceRegister->devreg[(*devSem)].t_transm_status;
        deviceRegister->devreg[(*devSem)].t_transm_command = ACK;

    }
    /* if terminal is in read */
    else{ 
        status = deviceRegister->devreg[(*devSem)].t_recv_status;
        deviceRegister->devreg[(*devSem)].t_recv_command = ACK;
        /* update devSem for the terminal read */
        (*devSem) = (*devSem) + DEVPERINT;
    }
    return(status);
    

}

/*helper method */
/* given an old state and a new state, this loops through all the registers in the old state 
and places them into the same registers but in the new state */
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



