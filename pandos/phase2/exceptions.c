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

void syscall(){

	state_t* exception_state = (state_t*) BIOSDATAPAGE;
	cpu_t current_time;
	storeState(exception_state, &(currentProc->p_s));

	/*update PC */
	currentProc->p_s.s_pc += 4;


	/*if in user-mode, trigger Program Trap Handler */
	if (currentProc->p_s.s_status & USERON != 0){
		ProgramTrapHandler();
	}



	/* if a0 = 1 */
	if (exception_state->s_a0 == CREATETHREAD){

	pcb_t *newProc = allocPcb();
	storeState(currentProc->p_s.s_a1, &(newProc->p_s));

	/*init newProc according to parameters in a0 and a1*/
	/*newProc->p_s = a1 p_s*/
	/*supportStruct*/
	
	insertChild(newProc, currentProc);
	storeState(currentProc->p_s.s_a2, newProc->p_supportStruct);
	insertProcQ(&readyQueue, newProc);
	processCount++;
	
	newProc->p_time = 0;
	newProc->p_semAdd = 0;
}

	

	/* if a0 = 2, terminate process recursively by calling freePcb*/
	if (exception_state->s_a0 = TERMINATETHREAD){
	debugSyscall(1, 2, 3, 4);
	terminateProcess(currentProc);
	scheduler();
	}

	/* if a0 = 3 , perform P*/
	
	

	if (exception_state->s_a0 == PASSERREN){
		
		
		/*physical address of semaphore goes in a1*/
		/* depending on value of semaphor, control is either returned to the current Process or process is blocked on ASL */

		/*first, decrement integer*/
		int semaphore = currentProc->p_s.s_a1;
		semaphore--;

		
		/*want to check if semaphore is < 0, assuming it is stored in a1)*/
		if (semaphore <= 0){
			insertBlocked(&semaphore, currentProc);
		}

		else{
			/*return control to the current process */
			contextSwitch(currentProc);
		}

	}

	/* if a0 = 4 */
	/*perform V */
		/*physical address of semaphore goes in a1*/
	

	debugVerhogen(1, 2, 3, 4);
	if (exception_state->s_a0 == VERHOGEN){
		
		debugA(1, 2, 3, 4);

		int semaphore = currentProc->p_s.s_a1;
		semaphore++;
	
		if (semaphore < 0){
			removeBlocked(semaphore);
			insertProcQ(&readyQueue, currentProc);
		}

		else{
			contextSwitch(currentProc);
		}
	}
		


	/* if a0 = 5 */
	
	

	if (exception_state->s_a0 = WAITIO){
		/* transitions from running to blocked state*/
		/*performs P on semaphore that nucleus maintains by values in a1, a2 and a3*/
		/*blocks current Process on ASL*/
		/*a1 is line # */
		/*a2 is device # */
		/*a3 is r/w */

		
		int lineNumber = currentProc->p_s.s_a1;
		int deviceNumber = currentProc->p_s.s_a2;
		int readWrite = currentProc->p_s.s_a3;

		
		/*check to make sure it is a valid line number */

		if (lineNumber > 3 && lineNumber < 7){
			/*get int that acts as Device semaphore*/
			/*this is what we need help with */

			int i = lineNumber * deviceNumber;

			/*if it is terminal, check to see if r/w is on and if so, treat as another device*/

			if (lineNumber = 7){
				if (readWrite = TRUE){
					i = i + 8;
				}

			}
			int semaphore = devSemaphore[i];

			/*perform P on it */

			/*check to see if it needs to be blocked*/
			if (semaphore <0){
				softBlockCount++;
				insertBlocked(&semaphore, currentProc);
			}

			/*if it does not need to be blocked, load new state and go */
			contextSwitch(currentProc);
		}
	}

	/* if a0 = 6 */
	if (exception_state->s_a0 == GETCPUTIME){
		/* get CPU time and place in v0 */
	cpu_t endTOD;
	cpu_t totaltime = endTOD + currentProc->p_time;
	currentProc->p_s.s_v0 = totaltime;

	/*do you have to do anything else to timer? */

	}

	/* if a0 = 7 */
	
	

	if (exception_state->s_a0 == WAITCLOCK){
		
		/*performs a P operation on pseudo-clock semaphore*/
		/*blocks current process on ASL */
		if (devSemaphore[SEM4DEV - 1] < 0){
			processCount++;
			insertBlocked(&(devSemaphore[SEM4DEV - 1]), currentProc);
		}
		
		scheduler();
	}

	/* if a0 = 8 */
	if (exception_state->s_a0 == GETSUPPORTPTR){
		/*requests a pointer to current process's support structure*/
		/*returns value of p_support struct */
		currentProc->p_s.s_v0 = (int) currentProc->p_supportStruct;
		return currentProc->p_s.s_v0;
	}

	/*if a0<1 */
	if (exception_state->s_a0 <= 1){
	ProgramTrapHandler(GENERALEXCEPT);
	}

	/*if a0>=9 */
	if (exception_state->s_a0 >= 9){
	ProgramTrapHandler(GENERALEXCEPT);
	}
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
	if (currentProcess->p_child = NULL){
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


void debugA(int a, int b, int c, int d){
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


