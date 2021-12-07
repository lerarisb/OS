/* Exceptions */

/*Written by Ben Leraris and Cole Shelgren */

/*this file contains methods to deal with syscall exceptions 
if you wake up here, the original exception code was 8

/*When the syscall assembly instruction is executed, the code enters the syscall handler below.
the syshandler determines the specific exception code and corresponding syscall method that needs to be called
the exception code is placed in the general purpose registers by the executing process 
there are different syscall methods for values 1-8 and any other values will go to the pass up or die method
there are also one helper method that helps terminate a process and all of its children 
and another helper method that helps block a process on the readyQueue */

/*this file references all global variables from initial */


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

void sysHandler(){

	/*saves exception state */
	state_PTR exception_state = (state_PTR) BIOSDATAPAGE;
	
	int lineTest = exception_state->s_a0;
	int isUserON = (exception_state->s_status & USERON);

	cpu_t current_time;

	/*store current state*/
	storeState(exception_state, &(currentProc->p_s));

	/*update PC */
	currentProc->p_s.s_pc = currentProc->p_s.s_pc + 4;


	/*if in user-mode, trigger Program Trap Handler */
	if (lineTest >= 1 && lineTest <= 8 && isUserON != 0){
		ProgramTrapHandler();
	}


	/* if a0 = 1 */
	if (lineTest == CREATETHREAD){
		SYSCALL1();
	}

	if (lineTest == TERMINATETHREAD){
		SYSCALL2();
	}

	/* if a0 = 3 , perform P*/
	if (lineTest == PASSERREN){
		SYSCALL3();
	}

	/* if a0 = 4 */
	/*perform V */
	if (lineTest == VERHOGEN){
		SYSCALL4();
	}

	/* if a0 = 5 */
	if (lineTest == WAITIO){
		SYSCALL5();
	}

	/* if a0 = 6 */
	if (lineTest == GETCPUTIME){
		SYSCALL6();
	}

	/* if a0 = 7 */
	if (lineTest == WAITCLOCK){
		SYSCALL7();
	}

	/* if a0 = 8 */
	if (lineTest == GETSUPPORTPTR){
		SYSCALL8();
	}

	/*if a0 is anything else */
	else{
	PassUpOrDie(GENERALEXCEPT);
	}
}




void SYSCALL1(){
	/*create new process */

	/*make space for new process */
	pcb_t *newProc = allocPcb();


	/*check to see if there was space for the new process */
	if (newProc != NULL){

		/*increase process count */
		processCount++;

		/*init newProc according to parameters in a0 and a1*/
		storeState((state_t*)currentProc->p_s.s_a1, &(newProc->p_s));
	
		/*check to see if there is a parameter in a2
		if no parameter provided, set field to null */


		if (((support_t*) currentProc->p_s.s_a2 != NULL) || ((support_t*) currentProc->p_s.s_a2 != 0)){
			newProc->p_supportStruct = (support_t*) currentProc->p_s.s_a2;}

		else{
			newProc->p_supportStruct = NULL;
		}
		
		currentProc->p_s.s_v0 = READY;

		/*insert process onto ready queue */
		insertProcQ(&readyQueue, newProc);

		/*insert newly created process as a child of current process */
		insertChild(currentProc, newProc);

	}

	else{
		currentProc->p_s.s_v0 = ERROR;
	}

	contextSwitch(currentProc);

	}


void SYSCALL2(){
	/*terminate a process and all of its children */
	terminateProcess(currentProc);
	scheduler();
	}


	
void SYSCALL3(){
	/*wait method */
	/*uses semaphores for mutual exclusions */

	/*physical address of semaphore goes in a1*/
	/* depending on value of semaphor, control is either returned to the current Process or process is blocked on ASL */
	/*first, decrement integer*/
	int *semaphore = (int*) currentProc->p_s.s_a1;
	(*semaphore)--;

		
	/*want to check if semaphore is < 0, assuming it is stored in a1)*/
	if ((*semaphore) < 0){

		/*if it is less than 0, insert current process onto readyQueue and
		store the time the process took and attribute it to the process in the 
		p_time field */
			
		helpBlocking(semaphore);
	}

	else{
		/*return control to the current process */
		contextSwitch(currentProc);
	}
}
	
void SYSCALL4(){

	/*go method*/
	/*uses sempahores for multual exclusion */
		
	pcb_t *temp;

	int *semaphore = (int *) currentProc->p_s.s_a1;
	(*semaphore)++;
	
	if ((*semaphore) <= 0){
		temp = removeBlocked(semaphore);
		if (temp != NULL){
			insertProcQ(&readyQueue, temp);
		}
	}
	contextSwitch(currentProc);
}
		

void SYSCALL5(){
	/*wait for IO */
	/* transitions from running to blocked state*/
		
		/*performs P on semaphore that nucleus maintains by values in a1, a2 and a3*/
		/*blocks current Process on ASL*/
		/*a1 is line # */
		/*a2 is device # */
		/*a3 is r/w */

		

		int lineNumber = currentProc->p_s.s_a1;


		int deviceNumber = currentProc->p_s.s_a2;
		int readWrite = currentProc->p_s.s_a3;
		
		/* do we need this ? */
		int i;

		
		/*check to make sure it is a valid line number */

		if ((lineNumber < DISKINT) || (lineNumber > TERMINT)){
			terminateProcess(currentProc);
			scheduler();
		}

		else{
			
			/*get int that acts as Device semaphore*/
			lineNumber = lineNumber - DISKINT;
			int i = lineNumber * DEVPERINT + deviceNumber;


			/*if it is terminal, check to see if r/w is on and if so, treat as another device*/

			if (lineNumber == TERMINT){
				if (readWrite = TRUE){
					i = i + DEVPERINT;
				}

			}

			/*decrement semaphore */

			devSemaphore[i]--;
			/*perform P on it */

			/*check to see if it needs to be blocked*/

			if (devSemaphore[i] < 0){
				softBlockCount++;
				helpBlocking(&(devSemaphore[i]));
			}

			else{
			/*if it does not need to be blocked, load new state and go */
			contextSwitch(currentProc);	
		}
	}
}


void SYSCALL6(){
	/* get CPU time and place in v0 */

	cpu_t stopClock;
	STCK(stopClock);

	/*charges time to current process */
	currentProc->p_time = (stopClock - startClock) + currentProc->p_time;
	currentProc->p_s.s_v0 = currentProc->p_time;

	/*starts clock again */
	STCK(startClock);

	/*returns control to executing process */
	contextSwitch(currentProc);

	}

void SYSCALL7(){
		
		/*performs a P operation on pseudo-clock semaphore*/
		/*blocks current process on ASL */
		
		devSemaphore[SEM4DEV - 1]--;

		if (devSemaphore[SEM4DEV - 1] < 0){
			softBlockCount++;
			helpBlocking(&(devSemaphore[SEM4DEV - 1]));
		}
		scheduler();
	}

void SYSCALL8(){
		/*requests a pointer to current process's support structure*/
		/*returns value of p_support struct */
		currentProc->p_s.s_v0 = (int) currentProc->p_supportStruct;
		contextSwitch(currentProc);
	}


/*occurs when uMPS3 fails in an attempt to translate a logical address into a corresponding address */
void TLBHandler(){
	PassUpOrDie(PGFAULTEXCEPT);
}

/*occurs when current process attempts to perform illegal or undefine action */
void ProgramTrapHandler(){
	PassUpOrDie(GENERALEXCEPT);
}

/*used when the exception is not 1-8 */
void PassUpOrDie(int exception){
	if (currentProc->p_supportStruct != NULL){
		
		/*store the saved exception state */
		storeState((state_t*) BIOSDATAPAGE, &(currentProc->p_supportStruct->sup_exceptState[exception]));
		
		/*pass control to the Nucleus Exception Handler, which we set the address of in initial */
		LDCXT(currentProc->p_supportStruct->sup_exceptContext[exception].c_stackPtr,
		currentProc->p_supportStruct->sup_exceptContext[exception].c_status, 
		currentProc->p_supportStruct->sup_exceptContext[exception].c_pc);

	}

	/*kills the executing process and all of its children */

	terminateProcess(currentProc);
	scheduler();

}




/*helper process for sys 2 
recursively terminates a given processes children and its children and so on */

void terminateProcess(pcb_t *currentProcess){
	
	/*continue terminating all processes until pointer to a process is null */
	while(!(emptyChild(currentProcess))){

		/*call terminate process on the child of the current process */
		terminateProcess(removeChild(currentProcess));
	}

	/*test to see if process that was given is the current process
	if so, remove the current process as a child from its parent */
	if(currentProcess == currentProc){
		outChild(currentProcess);
	}

	/*if there is not a semaphore for the given process
	remove it from the readyQueue */
	else if(currentProcess->p_semAdd == NULL){
		outProcQ(&readyQueue, currentProcess);
	}

	else{
		/*since it makes it here, we know the process has a semaphore and is blocked */
		/*remove the given process */
		pcb_t *temp = outBlocked(currentProcess);

		/*check to see if a process actually was removed */
		if (temp != NULL){
			int *semaphore = temp->p_semAdd;

			/*check to see if the process you are terminating was on the ASL
			if it was, decrease softBlockCount by 1 */

			if (temp >= (&devSemaphore[0]) && temp <= (&devSemaphore[SEM4DEV - 1])){
				softBlockCount--;
			}

			else{
				/*if terminated process is blocked on a semaphore, 
				the value of the semaphore must be adjusted */
				(*semaphore)++;
			}
		}
	}

	/*reflect that a process was terminated */
	processCount--;

	/*frees space to go with terminated process */
	freePcb(currentProcess);

}
/*helper method to block a process with a given semaphore */
void helpBlocking(int *semaphore){
	cpu_t TOD_stop;
	STCK(TOD_stop);
	currentProc->p_time = currentProc->p_time + (TOD_stop - startClock);
	insertBlocked(semaphore, currentProc);
	currentProc = NULL;
	scheduler();
}
