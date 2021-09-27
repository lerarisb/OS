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

/* pointer to the list of unused semaphore descriptors*/
semd_t *semdFree_h; 

/*pointer to the head of the linked list of used semaphore descriptors */
semd_t *semd_h; 

/*helper function that acts to search for given semaphore
helpTraverse takes a pointer to the address of a semaphore and checks to see if it is in the list of active semaphors
it returns a pointer to the matching semaphore*/

semd_t *helpTraverse(int *semAdd){
	/*store current semaphore and the next one to be used for traversal*/
	semd_t *currentSemd = semd_h;
	semd_t *nextSemd = semd_h->s_next;
	
	/*start at node after first dummy and check if the next semaphore
	 address is last node which is a dummy*/
	if ((int) nextSemd->s_semAdd == INT_MAX) {
		return semd_h;
	}
	
	/* if not at the last dummy, start traversing until you find the matching semaphore address
	because ASL is sorted by semAdd, keep traversing while semAdd is less than the next semAdd */
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



/*helper function to remove semaphor from active list
takes a pointer to a semaphore and adds it to the free list.  does not return anything*/
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

/* initialize the semdFree list, called once, takes no parameters and returns nothing */
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

/*helper function to allocate an active semaphor by removing from the free list
takes no parameters and returns a pointer to the newly removed semaphor*/

semd_t* allocSemd(){
	
	/*check to see if free list is null
	if it is, nothing can be removed*/
	if (semdFree_h == NULL){
		return NULL;
	}
	/* if there is space, remove a semaphor from the free list by removing the head  */
	semd_t *currentHead = semdFree_h;
	semdFree_h = currentHead->s_next;
	return currentHead;
}

/* given a pointer to a semaphore, semAdd, and a pointer to a pcb, insert the pcb at the tail end of semaphore address pointed to 
by semAdd*/
int insertBlocked(int *semAdd, pcb_t *p){

	/* variable that stores search results */
	semd_t *temp = helpTraverse(semAdd);
	
	/* if the semAdd is not in ASL, then need to add new semaphore */
	if(temp->s_next->s_semAdd != semAdd) {
		
		/* new semaphore being allocated */
		semd_t *newSemd = allocSemd();
		
		/* if new semaphore needs to be allocated and semdFree list is
		empty, return true */
		
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

/* given a pointer to a semaphore, remove the first pcb of process queue in the given semaphore descriptor */
pcb_t *removeBlocked(int *semAdd){
	
	/* variables to be used for pcb to be removed and the semaphore it is in */
	pcb_t* temp;
	semd_t *semaphore;
	semaphore = helpTraverse(semAdd);
	
	/* checks if semAdd is in list */
	if (semaphore->s_next->s_semAdd == semAdd){
		
		/* if semAdd is in the list, then call removeProcQ function from pcb.c to remove first pcb*/
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
			
			return temp;
		}
		else{
			return temp;
		}
	}
	/*semaphor was not in ASL so nothing to remove*/
	return NULL;
}

/* Does same thing as removeBlocked but calls outProcQ instead of removeProcQ
   Given a pointer p to a pcb, this removes that pcb and return the pointer to the removed pcb */
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
	/*semaphor was not in ASL so nothing to remove*/
	return NULL;
}

/* given a pointer to a semaphore, headblocked returns the pointer at head of process queue associated with the semaphore*/
pcb_t *headBlocked(int *semAdd){
	
	/* store semaphore at semAdd */
    semd_t *temp = helpTraverse(semAdd); 
    
    /* traversal does not return anything or procQ is empty */
    if(temp == NULL || emptyProcQ(temp->s_next->s_procQ)){
        return NULL;
    }
    
    /* otherwise, return the pointer to the head of the associated process queue by calling headProcQ function in pcb.c */
    else{
        return headProcQ(temp->s_next->s_procQ);
    }
}