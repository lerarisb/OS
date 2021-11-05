/* Exceptions */

/*for exception code 0, go to Device interrupt handler*/

/*for exception code 1-3, go to TLB exception handler*/

/*for codes 4-7 or 9-12, go to Program Trap exception handler*/

/*for code 8, go to syscall exception handler*/

void syscall(int exceptReason){

	
	/*increase pc by 4, since you have to do it in all syscalls */

	state_t *exceptState;
	exceptState->s_pc = (state_t*) 0x0FFF000
	exceptState->s_v0 = 1;
	exceptState->s_pc = exceptState->s_pc + 4;


	/*if in user-mode, trigger Program Trap Handler */


	/* if a0 = 1 */

	if (currentProc->p_s.s_a0 == 1){

	pcb_PTR *newProc = allocPcb();
	newproc->p_s = a1;

	/*init newProc according to parameters in a0 and a1*/
	/*newProc->p_s = a1 p_s*/
	/*supportStruct*/
	
	insertChild(newProc, currentProc);
	newProc ->p_supportStruct = 
	insertprocQ(readyQueue, newProc);
	processCount++;
	
	newProc->p_time = 0;
	newProc->p_semAdd = 0;
}


	/* if a0 = 2, terminate process recursively by calling freePcb*/
	
	


	if (currentProc->p_s.s_a0 = 2){

	pcb_PTR *temp = currentProc
	
	while (temp->p_child ! = NULL){
		temp = temp->p_child;
		while (temp->p_nextsib != NULL){
			temp = p->next_sib;
		}
	}
	freepcb(temp);
	processCount = processCount - 1;
		
		while (temp->p_parent != NULL){
		while (temp->prevSib != NULL){
			temp = temp->p_prevSib;
			freepcb(temp);
			processCount = processCount - 1;
		}
		temp = temp->p_parent;
	}
	freepcb(temp);
	processCount = processCount - 1;
	}

	
	scheduler()
	}

	/* if a0 = 3 , perform P*/
	if (currentProc->p_s.s_a0 == 3){
		/*physical address of semaphore goes in a1*/
		/* depending on value of semaphor, control is either returned to the current Process or process is blocked on ASL */

		/*first, decrement integer*/
		int semaphore = currentProc->p_s.s_a1
		semaphore--;

		
		/*want to check if semaphore is < 0, assuming it is stored in a1)*/
		if (semaphore <= 0){
			insertBlocked(&semaphore, currentProc);
		}

		else{
			/*return control to the current process */
			switchContext(currentProc);
		}

	}

	/* if a0 = 4 */
	/*perform V */
		/*physical address of semaphore goes in a1*/
	if (currentProc->p_s.s_a0 == 4){
		int semaphore = currentProc->p_s.s_a1;
		semaphore++;

		if (semaphore < 0){
			removeblocked(semaphore);
			insertprocQ(&readyQueue, currentProc);
		}

		else{
			switchContext(currentProc);
		}
		
	}

	/* if a0 = 5 */
	if (currentProc->p_s.s_a0 = 5){
		/* transitions from running to blocked state*/
		/*performs P on semaphore that nucleus maintains by values in a1, a2 and a3*/
		/*blocks current Process on ASL*/
		/*a1 is line # */
		/*a2 is device # */
		/*a3 is r/w */

		
		int lineNumber = currentProc->p_s.s_a1;
		int deviceNumber = currentProc->p_s.s_a2;
		int readWrite = currentProc->p_s.s_a3;

		/*need to figure out which device it is and then what is the associated semaphore */

		if (lineNumber == 1){

		}

		if (lineNumber == 2){


		}
		if (lineNumber == 3){

		}

		if (lineNumber == 4){


		}

		f (lineNumber == 5){

		}

		if (lineNumber == 6){


		}
		if (lineNumber == 7){

		}

		if (lineNumber == 8){


		}




		insertBlocked(&semaphore, currentProc);
		scheduler();

		nucleus performs a V operation on nucleus maintained semaphore whenever that subdevice generates an interrupt/*
		/*sub device's status word is returned in v0*/

		if (devSem[i] < 0){
			insertBlocked(&semaphore, currentProc);
		}

		else{
			/*return control to the current process */
			switchContext(currentProc);


	}

	/* if a0 = 6 */
	if (currentProc->p_s.s_a0 == 6){
		/* get CPU time and place in v0 */
	cput_t endTOD;
	cpu_t totaltime = endTOD + currentProc->p_time;
	currentProc->p_s.s_v0 = totaltime;

	/*do you have to do anything else to timer? */

	}

	/* if a0 = 7 */
	if (currentProc->p_s.s_a0 == 7){
		
		/*performs a P operation on pseudo-clock semaphore*/
		/*blocks current process on ASL */
		if (clockSem < 0){
			processCount++;
			insertBlocked(&clockSem, currentProc);
		}
		
		scheduler()
	}

	/* if a0 = 8 */
	if (currentProc->p_s.s_a0 = 8){
		/*requests a pointer to current process's support structure*/
		/*returns value of p_support struct */
		currentProc->p_s.s_v0 = p_supportStruct;
		return currentProc->p_s.s_v0;
	}
}



void devHandler(){

}

void TLBHandler(){

}

void ProgramTrapHandler(){

}





