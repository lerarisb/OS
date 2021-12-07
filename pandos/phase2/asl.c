/*
asl.c maintains a sorted NULL-terminated single, linearly linked list (using the field s_next) of semaphore descriptors whose head 
is pointed by the variable semd_h.  The list semd_h points to represents the Active Semaphore List (ASL), which is sorted in ascending
order using the s_semdAdd field as the sort key.

this file also maintains a second list lof semaphore descriptors, the semdFree list, to hold the unused semaphore descriptors.  
This list, whose head is pointed to by the variable semdFree_h.  The free list is kept, like the pcbFree list, 
as a NULL-terminated single, linearly linked list (using the s_next field).

the semaphore descriptors are declared as a static array of size MAXPROC + 2 of type semd_t.  
The array has 2 dummy nodes, one at the beginning and one at the end

this file supports the insert, removal, and initizalization of the ASL through the following externally visible functions;
insertBlocked, removeBlocked, outBlocked, headBlocked, and initASL

the file also contains helper functions called helpTraverse and freeSmd
*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include <limits.h>

semd_t *semdFree_h; 
semd_t *semd_h; 

/*helper function that acts to search for given semaphore*/
semd_t *helpTraverse(int *semAdd){
	/*store current semaphore and the next one to be used for traversal*/
	semd_t *currentSemd = semd_h;
	
	
	/* if not at the last dummy start traversing until you find the matching semaphore
	address */
	while (semAdd > currentSemd->s_next->s_semAdd){
		/*checks to see if current semaphore address is one we are looking for*/
		currentSemd = currentSemd->s_next;
	}
	return currentSemd;

}


/*helper function to remove semaphor from active list*/
void freeSemd(semd_t *semd) {
	/* checks if free list is empty, if it is, then add semd to free list and set it 
	as pointer */
	if(semdFree_h == NULL){
		return NULL;
	}
	/* if it is not empty add semd to the tail and change the head */
	else {
		semd->s_next = NULL;
		semd -> s_semAdd = NULL;
		semd -> s_procQ = mkEmptyProcQ();
		return semd;
	}
}

/* initialize the semdFree list */
void initASL(){
	/* initialize the semdTable */
	static semd_t semdTable[MAXPROC+2];
	semd_h = NULL;
	semdFree_h = &semdTable[0];
	int i;
	
	/* fill the table excluding the two dummy nodes */
	for(i=0; i < (MAXPROC+2); i++){
		semdTable[i].s_next = &semdTable[i+1];
	}	

	/* initializing the two dummy nodes */
	semd_t *dumb;
	semd_t *dumber;

	/* making dumber the last dummy node */

	dumber = &(semdTable[MAXPROC+1]);
    dumber->s_semAdd = NULL; 
    dumber->s_procQ = mkEmptyProcQ();
    dumber->s_next = NULL;

/*making dumb the first dummy node */
	dumb = &(semdTable[MAXPROC]);
	dumb->s_semAdd = 0;
    dumb->s_procQ = mkEmptyProcQ();
    dumb->s_next = dumber;

    /*make first dummy node the head */
    semd_h = dumb; 
	
}


/*helper function to allocate active semaphor*/
/*remove a semaphor from free list*/
/*return newly removed semaphor*/
semd_t* allocSemd(semd_PTR s, int *semAdd){
	/*check to see if free list is null*/
	if (semdFree_h == NULL){
		return NULL;
	}
	/* deallocate space from free to ASL */
	
	semd_t *currentHead = semdFree_h;

	/*removing by setting head equal to the next free semaphore */
	semdFree_h = semdFree_h->s_next;

	/*remove given semaphore by setting pointers to go around it */
	currentHead->s_next = s->s_next;
	s-> s_next = currentHead;

	/*emptying processes of current head */
	currentHead->s_procQ = mkEmptyProcQ();

	/*setting semaphore equal to the given int */
	currentHead-> s_semAdd = semAdd;

	return currentHead;
}


/* insert pcb point to by p at tail end of semaphore address pointed to 
by semAdd*/
int insertBlocked(int *semAdd, pcb_t *p){
	/* variable that stores search results */

	
	semd_t *temp = helpTraverse(semAdd);
	semd_t *newSemd = temp->s_next;
	/* if the semAdd is not in ASL, then need to add new semaphore */
	
	if(temp->s_next->s_semAdd != semAdd) {
		
		/* new semaphore being allocated */
		
		newSemd = allocSemd(temp, semAdd);
		/* if new semaphore needs to be allocated and semdFree list is
		empty return true */

	}
		
	if(newSemd == NULL){
		return TRUE;
		}
		/*otherwise return false */

	p->p_semAdd = semAdd;
	insertProcQ(&(newSemd->s_procQ), p);
	return FALSE;

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
			toBeRemoved -> s_next = semdFree_h;
			semdFree_h = toBeRemoved;
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

