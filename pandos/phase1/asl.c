#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
/* #include "../h/asl.h" */
/* #include "../phase1/pcb.c" */
#include <limits.h>

semd_t *semdFree_h; 
semd_t *semd_h; 

/*helper function that acts to search for given semaphore*/
semd_t *helpTraverse(int *semAdd){
	/*start at head of active list*/
	semd_t *currentSemd = semd_h;
	semd_t *nextSemd = semd_h->s_next;
	/*first case head of active list is the given semaphore*/
	if (nextSemd->s_semAdd == (INT_MAX)) {
		return semd_h;
	}
	/*head is not given semaphore so must traverse*/
	/*checks to make sure not at end of list*/
	while (semAdd >= nextSemd->s_semAdd){
		/*checks to see if current semaphore is given semaphore*/
		if (nextSemd->s_semAdd == semAdd){
			return currentSemd;
		}
		currentSemd = nextSemd;
		nextSemd = nextSemd->s_next;
	}
	return currentSemd;
}

pcb_t *headBlocked(int *semAdd){
    semd_t *temp;
    temp = helpTraverse(semAdd); 
    if(temp == NULL || emptyProcQ(temp->s_procQ)){
        return NULL;
    }
    else{
        return headProcQ(temp->s_procQ);
    }
}

/*helper function to remove semaphor from active list*/
void freeSemd(semd_t *semd) {
	/* checks if free list is empty, if it is, then add semd to free list and set it as pointer */
	if(semdFree_h == NULL){
		semd->s_next = NULL;
		semdFree_h == NULL;
	}
	/* if it is not empty add semd to the tail and change the head */
	else {
		semd->s_next = semdFree_h;
		semdFree_h = semd;
	}
}

void initASL(){
	/* initialize the semdTable */
	static semd_t semdTable[MAXPROC+2];
	semd_h = NULL;
	semdFree_h = NULL;
	int i;
	/* fill the table excluding the two dummy nodes */
	for(i=2; i < (MAXPROC+2); i++){
		freeSemd(&(semdTable[i]));
	}
	
	/* initializing the two dummy nodes */
	semd_t *dumb;
	dumb = &(semdTable[0]);
	semd_t *dumber;
	dumber = &(semdTable[1]);
	
	/* making dumb the first dummy node */
	dumb->s_semAdd = 0;
    	dumb->s_procQ = NULL;
    	dumb->s_next = dumber;
    	
    	/* making dumber the last dummy node */
    	dumber->s_semAdd = (INT_MAX); 
    	dumber->s_procQ = NULL;
    	dumber->s_next = NULL;

}

/*helper function to allocate active semaphor*/
/*remove a semaphor from free list*/
/*return newly removed semaphor*/

semd_t* semAlloc(){

	/*check to make sure free list is not null*/

	if (semdFree_h == NULL){
		return NULL;
	}
	semd_t *currentHead = semdFree_h;
	semdFree_h = currentHead->s_next;
	return currentHead;
}

int insertBlocked(int *semAdd, pcb_t *p){
	semd_t *temp = helpTraverse(semAdd);
	if(temp->s_next->s_semAdd != semAdd) {
		semd_t *newPcb = semAlloc();
		if(newPcb == NULL){
			return TRUE;
		}
		else {
			newPcb->s_next = temp->s_next;
			temp->s_next = newPcb;
			newPcb->s_procQ = mkEmptyProcQ();
			p->p_semAdd = semAdd;
			newPcb->s_semAdd = semAdd;
			insertProcQ(&(newPcb->s_procQ), p);
			return FALSE;
		}	
	}
	else{
		p->p_semAdd = semAdd;
		insertProcQ(&(temp->s_next->s_procQ), p);
		return FALSE;
	}		
}


pcb_t *removeBlocked(int *semAdd){
	pcb_t* temp;
	semd_t *semaphore;
	semaphore = helpTraverse(semAdd);
	/* checks if semAdd is in list */
	if (helpTraverse(semAdd) != NULL){
		temp = removeProcQ(&semaphore->s_next->s_procQ);
	

		/*check to see if a pcb can be removed*/
		/*if the process queue for semaphore is empty, all pcbs are gone */
		/*remove the descriptor from ASL and return to semdFree list*/
		if (emptyProcQ(semaphore->s_next->s_procQ)){
			semd_t *toBeRemoved = semaphore->s_next;
			semaphore->s_next = semaphore->s_next->s_next;
			freeSemd(toBeRemoved);
			temp->p_semAdd = NULL;
			return temp;
		}
		else{
			temp->p_semAdd = NULL;
			return temp;
		}
	}

	/*semaphor was not in ASL so nothing to remove*/
	return NULL;

}

pcb_t *outBlocked(pcb_t *p){
	pcb_t* temp;
	semd_t *semaphore;
	semaphore = helpTraverse(p->p_semAdd);
	/* checks if semAdd is in list */
	if (semaphore != NULL){
		temp = outProcQ(&semaphore->s_next->s_procQ, p);
	

		/*check to see if a pcb can be removed*/
		/*if the process queue for semaphore is empty, all pcbs are gone */
		/*remove the descriptor from ASL and return to semdFree list*/
		if (emptyProcQ(semaphore->s_next->s_procQ)){
			semd_t *toBeRemoved = semaphore->s_next;
			semaphore->s_next = semaphore->s_next->s_next;
			freeSemd(toBeRemoved);
			temp->p_semAdd = NULL;
			return temp;
		}
		else{
			temp->p_semAdd = NULL;
			return temp;
		}
	}

	/*semaphor was not in ASL so nothing to remove*/
	return NULL;

}


