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
	/*store current semaphore and the next one to be used for traversal*/
	semd_t *currentSemd = semd_h;
	semd_t *nextSemd = semd_h->s_next;
	/*start at node after first dummy and check if the next semaphore
	 address is last node which is a dummy*/
	if ((int) nextSemd->s_semAdd == INT_MAX) {
		return semd_h;
	}
	/* if not at the last dummy start traversing until you find the matching semaphore
	address */
	while (semAdd >= nextSemd->s_semAdd){
		/*checks to see if current semaphore address is one we are looking for*/
		if (nextSemd->s_semAdd == semAdd){
			return currentSemd;
		}
		/* continue traversing until found */
		currentSemd = nextSemd;
		nextSemd = nextSemd->s_next;
	}
	return currentSemd;
}



/*helper function to remove semaphor from active list*/
void freeSemd(semd_t *semd) {
	/* checks if free list is empty, if it is, then add semd to free list and set it 
	as pointer */
	if(semdFree_h == NULL){
		semd->s_next = NULL;
		semdFree_h = semd;
	}
	/* if it is not empty add semd to the tail and change the head */
	else {
		semd->s_next = semdFree_h;
		semdFree_h = semd;
	}
}

/* initialize the semdFree list */
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
    /*make first dummy node the head */
    semd_h = dumb;   	
    /* making dumber the last dummy node */
    dumber->s_semAdd = (INT_MAX); 
    dumber->s_procQ = NULL;
    dumber->s_next = NULL;
}

/*helper function to allocate active semaphor*/
/*remove a semaphor from free list*/
/*return newly removed semaphor*/
semd_t* allocSemd(){
	/*check to see if free list is null*/
	if (semdFree_h == NULL){
		return NULL;
	}
	/* deallocate space from free to ASL */
	semd_t *currentHead = semdFree_h;
	semdFree_h = currentHead->s_next;
	return currentHead;
}

/* insert pcb point to by p at tail end of semaphore address pointed to 
by semAdd*/
int insertBlocked(int *semAdd, pcb_t *p){
	/* variable that stores search results */
	semd_t *temp = helpTraverse(semAdd);
	/* if the semAdd is not in ASL, then need to add new semaphore */
	if(temp->s_next->s_semAdd != semAdd) {
		/* new semaphore being allocated */
		semd_t *newSemd = allocSemd();
		/* if new semaphore needs to be allocated and semdFree list is
		empty return true */
		if(newSemd == NULL){
			return TRUE;
		}
		/* in all other cases return false */
		else {
			/* add new semaphore to active list */
			newSemd->s_next = temp->s_next;
			temp->s_next = newSemd;
			/* get pointer of empty process queue */
			newSemd->s_procQ = mkEmptyProcQ();
			/* set semaphore address of for p and semaphore*/
			p->p_semAdd = semAdd;
			newSemd->s_semAdd = semAdd;
			/* insert p into pcb */
			insertProcQ(&(newSemd->s_procQ), p);
			return FALSE;
		}	
	}
	/* if semaphore address was found */
	else{
		/* set semaphore address */
		p->p_semAdd = semAdd;
		/* insert p into pcb */
		insertProcQ(&(temp->s_next->s_procQ), p);
		return FALSE;
	}		
}

/* remove the first pcb of process queue in semaphore descriptor pointed to 
by semAdd */
pcb_t *removeBlocked(int *semAdd){
	/* variables to be used for pcb to be removed and semaphore it is in */
	pcb_t* temp;
	semd_t *semaphore;
	semaphore = helpTraverse(semAdd);
	/* checks if semAdd is in list */
	if (semaphore->s_next->s_semAdd == semAdd){
		/* if in list then call removeProcQ function from pcb.c */
		temp = removeProcQ(&semaphore->s_next->s_procQ);
		/*if the process queue for semaphore is empty, all pcbs are gone and need 
		to remove the descriptor from ASL and return to semdFree list*/
		if (emptyProcQ(semaphore->s_next->s_procQ)){
			/* get semaphore that will be removed */
			semd_t *toBeRemoved = semaphore->s_next;
			/* unlink semaphore from ASL */
			semaphore->s_next = semaphore->s_next->s_next;
			/* remove semaphore and set its address to null */
			freeSemd(toBeRemoved);
			temp->p_semAdd = NULL;
			return temp;
		}
		else{
			temp->p_semAdd = NULL;
			return temp;
		}
	}
	else{
		/*semaphor was not in ASL so nothing to remove*/
		return NULL;
	}
}

/* Does same thing as removeBlocked but calls outProcQ instead of removeProcQ */
/* remove a pcb that is not the tail */
pcb_t *outBlocked(pcb_t *p){
	/* variables to be used for pcb to be removed and semaphore it is in */
	pcb_t* temp;
	semd_t *semaphore;
	semaphore = helpTraverse(p->p_semAdd);
	/* checks if semAdd is in list */
	if (semaphore->s_next->s_semAdd == p->p_semAdd){
		/* if in list then call outProcQ function from pcb.c */
		temp = outProcQ(&semaphore->s_next->s_procQ, p);
		/*if the process queue for semaphore is empty, all pcbs are gone and need 
		to remove the descriptor from ASL and return to semdFree list*/
		if (emptyProcQ(semaphore->s_next->s_procQ)){
			/* get semaphore that will be removed */
			semd_t *toBeRemoved = semaphore->s_next;
			/* unlink semaphore from ASL */
			semaphore->s_next = semaphore->s_next->s_next;
			/* remove semaphore and set its address to null */
			freeSemd(toBeRemoved);
			return temp;
		}
		else{
			temp->p_semAdd = NULL;
			return temp;
		}
	}
	else{
		/*semaphor was not in ASL so nothing to remove*/
		return NULL;
	}
}

/* returns pointer at head of process queue associated with the semaphore at
semAdd*/
pcb_t *headBlocked(int *semAdd){
	/* store semaphore at semAdd */
    semd_t *temp = helpTraverse(semAdd); 
    /* traversal does not return anything or procQ is empty */
    if(temp == NULL || emptyProcQ(temp->s_next->s_procQ)){
        return NULL;
    }
    /* otherwise return pointer to head of process queue by calling 
    headProcQ function in pcb.c */
    else{
        return headProcQ(temp->s_next->s_procQ);
    }
}