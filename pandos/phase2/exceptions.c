/* Exceptions */

/*for exception code 0, go to Device interrupt handler*/

/*for exception code 1-3, go to TLB exception handler*/

/*for codes 4-7 or 9-12, go to Program Trap exception handler*/

/*for code 8, go to syscall exception handler*/


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
extern pcb_t*readyQueue;
extern cpu_t startClock;
extern int devSemaphore[SEM4DEV];

void sysHandler(){


	state_t* exception_state = (state_t*) BIOSDATAPAGE;
	
	int lineTest = exception_state->s_a1;
	debugline(lineTest);

	cpu_t current_time;
	storeState(exception_state, &(currentProc->p_s));

	/*update PC */
	currentProc->p_s.s_pc += 4;


	/*if in user-mode, trigger Program Trap Handler */
	if (currentProc->p_s.s_status & USERON != 0){
		debugUM(1,2,3,4);
		ProgramTrapHandler();
	}

	

	/* if a0 = 1 */
	if (exception_state->s_a0 == CREATETHREAD){
		SYSCALL1();}

	if (exception_state->s_a0 == TERMINATETHREAD){
		SYSCALL2();
	}

	/* if a0 = 3 , perform P*/
	if (exception_state->s_a0 == PASSERREN){
		SYSCALL3();
	}

	/* if a0 = 4 */
	/*perform V */
	if (exception_state->s_a0 == VERHOGEN){
		SYSCALL4();
	}

	/* if a0 = 5 */
	if (exception_state->s_a0 == WAITIO){
		SYSCALL5();
	}

	/* if a0 = 6 */
	if (exception_state->s_a0 == GETCPUTIME){
		SYSCALL6();}

	/* if a0 = 7 */
	if (exception_state->s_a0 == WAITCLOCK){
		SYSCALL7();
	}

	/* if a0 = 8 */
	if (exception_state->s_a0 == GETSUPPORTPTR){
		SYSCALL8();}

	/*if a0 is anything else */
	else{
	ProgramTrapHandler(GENERALEXCEPT);
	}


}


void SYSCALL1(){
	
	/*create new process */
	pcb_t *newProc = allocPcb();


	/*check to see if there was space for the new process */
	if (newProc != NULL){

		/*increase process count */
		processCount++;

		/*init newProc according to parameters in a0 and a1*/
		storeState((state_t*)currentProc->p_s.s_a1, &(newProc->p_s));
	
		/*check to see if there is a parameter in a2
		if no parameter provided, set field to null */

		if ((support_t*) currentProc->p_s.s_a2 == NULL){
			newProc->p_supportStruct = NULL;}

		else{
			/*if parameter is provided, set Support Struct equal to parameter */
			newProc->p_supportStruct = (support_t*) currentProc->p_s.s_a2;
		}

	/*process has yet to accumulate any cpu time */
	newProc->p_time = 0;

	/*this process is in the ready state */
	newProc->p_semAdd = 0;

	/*return value of 0 in caller's v0 */
	currentProc->p_s.s_v0 = 0;

	/*insert process onto ready queue */
	insertProcQ(&readyQueue, newProc);

	/*insert newly created process as a child of current process */
	insertChild(newProc, currentProc);

}

else{
	/*if there is no room in the pool to create process
	return -1 in caller's v0
	and return control to current process */

	currentProc->p_s.s_v0 = -1;
	}

contextSwitch(currentProc);

}

void SYSCALL2(){
	debugSyscall(1, 2, 3, 4);
	terminateProcess(currentProc);
	scheduler();
	}


	
void SYSCALL3(){

		
	
		/*physical address of semaphore goes in a1*/
		/* depending on value of semaphor, control is either returned to the current Process or process is blocked on ASL */

		/*first, decrement integer*/
		int *semaphore = (int*) currentProc->p_s.s_a1;
		semaphore--;

		
		/*want to check if semaphore is < 0, assuming it is stored in a1)*/
		if ((*semaphore) < 0){

			/*if it is less than 0, insert current process onto readyQueue and
			store the time the process took and attribute it to the process in the 
			p_time field */
			
			helpBlocking(semaphore);

		}

		else{
			debugA(1,2,3,4);
			/*return control to the current process */
			contextSwitch(currentProc);
		}

	}
	
void SYSCALL4(){
		
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
		/* transitions from running to blocked state*/
		/*performs P on semaphore that nucleus maintains by values in a1, a2 and a3*/
		/*blocks current Process on ASL*/
		/*a1 is line # */
		/*a2 is device # */
		/*a3 is r/w */

		

		int lineNumber = currentProc->p_s.s_a1;

		debugline(lineNumber);

		int deviceNumber = currentProc->p_s.s_a2;
		int readWrite = currentProc->p_s.s_a3;
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

			devSemaphore[i]--;
			debugSemaphore(devSemaphore[i]);

			

			/*perform P on it */

			/*check to see if it needs to be blocked*/

			if (devSemaphore[i] < 0){
				debugNeedToBeBlocked(1, 2, 3, 4);
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
	cpu_t current_time;
	STCK(current_time);
	current_time = (current_time - startClock) + currentProc -> p_time;
	currentProc->p_s.s_v0 = current_time;
	contextSwitch(currentProc);

	/*do you have to do anything else to timer? */

	}

void SYSCALL7(){
		
		/*performs a P operation on pseudo-clock semaphore*/
		/*blocks current process on ASL */
		
		devSemaphore[SEM4DEV - 1]--;

		if (devSemaphore[SEM4DEV - 1] < 0){
			processCount++;
			helpBlocking(&(devSemaphore[SEM4DEV - 1]));
		}
		contextSwitch(currentProc);
	}

void SYSCALL8(){
		/*requests a pointer to current process's support structure*/
		/*returns value of p_support struct */
		currentProc->p_s.s_v0 = (int) currentProc->p_supportStruct;
		return currentProc->p_s.s_v0;
	}



void TLBHandler(){
	PassUpOrDie(PGFAULTEXCEPT);
}

void ProgramTrapHandler(){
	PassUpOrDie(GENERALEXCEPT);
}

void PassUpOrDie(int exception){
	if (currentProc->p_supportStruct != NULL){
		
		/*store the saved exception state */
		 
		storeState((state_t*) BIOSDATAPAGE, &(currentProc->p_supportStruct->sup_exceptState[exception]));
		
		/*pass control to the Nucleus Exception Handler, which we set the address of in initial */
		LDCXT(currentProc->p_supportStruct->sup_exceptContext[exception].c_stackPtr,
		currentProc->p_supportStruct->sup_exceptContext[exception].c_status, 
		currentProc->p_supportStruct->sup_exceptContext[exception].c_pc);

	}

}





void terminateProcess(pcb_t *currentProcess){
	
	/*base case*/
	if (currentProcess->p_child == NULL){
		currentProcess = currentProcess->p_prnt;
		freePcb(removeChild(currentProcess));

	}

	else{
		while (currentProcess->p_child != NULL){
			currentProcess = currentProcess->p_child;
		}
	
			removeChild(currentProcess);
			terminateProcess(currentProcess);
	}
}

void helpBlocking(int *semaphore){
	cpu_t TOD_stop;
	STCK(TOD_stop);
	currentProc->p_time = currentProc->p_time + (TOD_stop - startClock);
	insertBlocked(semaphore, currentProc);
	currentProc = NULL;
	scheduler();
}




void debugA(int a, int b, int c, int d){
	a = 42;
	b = 21;
}

void debugUM(int a, int b, int c, int d){
	a = 42;
	b = 21;
}

void debugVerhogen(int a, int b, int c, int d){
	a = 42;
	b = 21;
}

void debugSyscall(int a, int b, int c, int d){
	a = 42;
	b = 21;
}

void debugRegister(state_t *test){
	int i = 1;
}

void debugNeedToBeBlocked(int a, int b, int c, int d){
	a++;
}

void debugSemaphore(int a){
	int i = a;
}

void debugWAITIO(int a, int b, int c, int d){
	a++;
}

void debugline(int a){
	int b = a;
}

void debugSyscall1(int a, int b, int c, int d){
	a++;
}


