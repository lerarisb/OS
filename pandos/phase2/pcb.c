/*this file contains ways to interact with the data structures that represent processes, also called process control blocks or pcbs
pcb.c acts as a queue manager and implements four pcb related sets of functions
it supports allocation and deallocation of pcbs, maintenance of queues of pcbs, maintenance of trees of pcbs, 
maintenance of a single sorted list of active semaphore descriptors, each of which supports a queue of pcbs: the ASL

#here is a pool of size MAXPROC pcbs which can be allocated from and deallocated to
the free or unused ones are kept on a null terminated single, linearly linked list (using the p_next field) called the pcbFree list
whose head is pointed to by the variable pcbFree_h

this file contains a method that initalizes a list of 20 pcbs 

the file also contains generic queue manipulation methods where one of the parameters 
is a pointer to the queue upon which the indicated operation is to be performed

the queue of pcbs to be manipulated, which are called process queues, are all double, circularly linked list that use the
p_next and p_prev fields, where each queue is pointed at by a tail pointer

the generic queue manipulation methods used in this file mkEmptyProcQ, emptyProcQ, removeProcQ, outProcQ, headProcQ

pcbs are also organized into trees of pcbs, called process queues, that use the pointers p_prnt, p_child, and p_sib
a parent pcb contains a pointer, p_child, to a null terminated single, lineraly linked list of its child pcbs
each child process has a pointer to its parent pcb and possible the next child pcb of its same parent 

to support process trees there are the following 4 externally visible methods: emptyChild, insertChild, Remove Child, Outchild

*/

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"



/*pointer to the head of the free list*/
HIDDEN pcb_PTR pcbFree_h;

/*given a pointer to a pcb, p, insert p onto the pcbFree list
    since pcb is no longer in use, it is being removed from the queue.  nothing is returned*/ 
void freePcb(pcb_PTR p){
    insertProcQ(&pcbFree_h, p);
}

/*allocated a pcb from the free list to the queue of active pcbs
this method takes no paramteters and returns a pointer to the pcb that was just allocated onto the active queue */
pcb_PTR allocPcb(){
    if (pcbFree_h == NULL){
        return NULL;
    }
    /*remove the first element from pcb free and add to queue
    removes any previous values from pcb*/
    pcb_PTR temp;
    temp = removeProcQ(&pcbFree_h);


    /*all previous values of pcb are set to null*/
    temp->p_child = NULL;
    temp->p_sib = NULL;
    temp->p_prnt = NULL;
    temp->p_next = NULL;
    temp->p_prev = NULL;
    temp->p_prevSib = NULL;
    temp->p_semAdd = NULL;
    temp->p_time = 0;

return temp;
}

/*initalizes the pcbFree list to contain all the elements of the static array of MAXPROC pcbs
only called once during data initalization
initPcbs takes no parameters and returns nothing*/
void initPcbs(){
    static pcb_t pcbInit[MAXPROC];
    pcbFree_h = NULL;
    int i = 0;
    while(i < MAXPROC){
        insertProcQ(&pcbFree_h, &pcbInit[i]);
        i++;
    }
}

/* returns pointer to the tail of an empty process queue
takes no parameters and returns a null pointer to a pcb */
pcb_PTR mkEmptyProcQ(){
    return NULL;
}

/* given a pointer to a process queue, determines if that process queue is empty by returning true or false accordingly */
int emptyProcQ(pcb_PTR tp){
    return (tp == NULL);
}

/*given the tail pointer of the process queue and a pointer to a pcb, inserts the pcb into the process queue*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p){
    /* checks if process queue is empty 
    if it is empty, make the queue circular */
    if(emptyProcQ((*tp))){
        p->p_next = p;
        p->p_prev = p;
    }
    else {


        /* store current tail */
        pcb_PTR currentTail = *tp;
        
        /* set new node's next to be equal to currentTail's next */ 
        p->p_next = currentTail->p_next;
        
        /* set currentTail's next to point to newly inserted node */
        currentTail->p_next = p;
        
        /* set new node's previous to point to current tail */
        p->p_prev = currentTail;
        
        /* setting the node pointed to by p_next so that p_prev points back to p */
        p->p_next->p_prev = p;
    
        
    }
    /* set tail pointer to new node */
    *tp = p;

}

/*given the tail pointer to a process queue, remove the node from the head of that process queue
returns a pointer to the removed pcb*/
pcb_PTR removeProcQ(pcb_PTR *tp){
    /*check to see if the given process queue is empty
    if there are no nodes in the queue, the head cannot be removed*/
    if (emptyProcQ(*tp)){
        return NULL;
    }
    /*send the given tail pointer to the process queue, along with the pcb at the had of that queue, to outProcQ*/
    return (outProcQ(tp, (*tp)->p_next));
}

/*given a pointer to the tail of a process queue and a pointer to a specific pcb, 
this removes the specified pcb (p) from the queue pointed to by (tp)
returns a pointer to the removed pcb*/
pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p){
  pcb_PTR current = (*tp)->p_next;
   
    /*first, check to see if the given process queue pointed to by tp is empty
    if the given process queue contains no pcbs, none can be removed.  so null is returned*/
   if (emptyProcQ(*tp)){
            return NULL;
   }

   /*loop through all pcbs in the process queue
   if the current pcb is not the given pcb, and if the current pcb is not the tail pointer (meaning not the end of the queue)
   then the loop continues*/

    while (p != current && current != *tp){
        current = current->p_next;
    }
    /*when the current pcb is the given pcb that we want to remove*/
    if (current == p){
        current->p_next->p_prev = current->p_prev;
        current->p_prev->p_next = current->p_next;
        if(current == *tp){
            *tp = current->p_prev;
        }
        if (p->p_next == p){
            *tp = NULL;
        }
        p->p_next = NULL;
        p->p_prev = NULL;
        return p;
    }
    /*if code gets here, pcb is not in the process queue so we return null*/
    return NULL;
}

/*given a tail pointer to a process queue, return a pointer to the first pcb from that process queue */
pcb_PTR headProcQ(pcb_PTR tp){
    
    /*checks to see if given process queue is empty*/
    if (emptyProcQ(tp)){
        return NULL;
    }
    /*if not empty, returns head of process queue*/
    return (tp->p_next); 
}

/*given a pointer to a pcb, this determines if that pcb has any children*/
int emptyChild(pcb_PTR p){
    return (p->p_child == NULL);
}

/*given a pointer to a pcb p and a pointer to a parent pcb, prnt, this adds the pcb p as a child to the pcb prnt
does not return anything */
void insertChild(pcb_PTR prnt, pcb_PTR p){
    
    /*check to see if prnt already has a child*/
    if (emptyChild(prnt)){
    /*if parent does not have a child, add the pcb as one*/
        prnt->p_child = p;
        /* set parent and siblings of new child */
        p->p_prnt = prnt;
        p->p_sib = NULL;
        p->p_prevSib = NULL;
    }    
    /* if the parent already has a child */    
    else{
        /*if parent has a child, add pcb as child and move existing children*/
        pcb_PTR firstChild =  prnt->p_child;
        /*set new pcb as child of parent*/
        prnt->p_child = p;
        /* set the previousSib of the old first child to point to new child */
        firstChild->p_prevSib = p;
        /*set the new child's parent to be given parent pcb*/
        p->p_prnt = prnt;
        /*set the inserted pcb so that it's sibling is the original first child*/
        p->p_sib = firstChild;
        /* set the newly inserted child so that it's prev sib is null */
        p->p_prevSib = NULL;
    }
}

/*given a pointer to a pcb p, remove the first child of p
removechild returns a pointer to the pcb that was removed*/
pcb_PTR removeChild(pcb_PTR p){
    /* store the first child of p for future use *
    pcb_PTR currentChild = p->p_child;
    
    /*test to see if p has child
    if p has no child, return null*
    if (currentChild == NULL){
        return NULL;
    }
    /* if there is only one child of p *
    else if (currentChild->p_sib == NULL){
        /*take the only child and set its parent to null*
        currentChild->p_prnt = NULL; 
        /*take the parent and set its child to null*
        p->p_child = NULL;
        return currentChild;
    }
    /*if p has more than one child*
    else{
        
        /* set p's child to point to the current child's sibling *
        p->p_child = currentChild->p_sib;
        
        /* set parent and sibling of current child to null *
        currentChild->p_prnt = NULL;
        currentChild->p_sib = NULL;
        
        /* set prevSib of new child to be null *
        p->p_child->p_prevSib = NULL;

        return currentChild;
    }   */


if (emptyChild(p)){
        return (NULL);
    }
    /*one child exists*/
    else {
        pcb_PTR temp = p -> p_child;
        /*checks if there is a sibling - if not only child*/
        if (p-> p_child -> p_sib == (NULL)){
            /*child is removed and set as null*/
            p -> p_child = (NULL);
            temp -> p_prnt = (NULL);
            temp -> p_sib = (NULL);
            temp -> p_prevSib = (NULL);
            return (temp);}
        /*more than one child*/
        else{
            /*the child is removed and p now points at the sibling*/
            p -> p_child = p -> p_child -> p_sib;
            /*there is no longer a previous sibling*/
            p -> p_child -> p_prevSib = (NULL);
            p -> p_prnt = (NULL);
            p -> p_sib = (NULL);
            p -> p_prevSib = (NULL);
            return (temp);}
    }







}

/*given a pointer p to a child pcb, this removes p
this returns a pointer to the pcb that was just removed */
pcb_PTR outChild(pcb_PTR p){
   /*store parent of p for further use *
    pcb_PTR parent = p->p_prnt;
    if (p == NULL){
        return NULL;
    }
    /*check to see if p is actually a child and has an existing parent*
    if (parent == NULL){
        return NULL;
    }
    /* if p is the only child of its parent *
    if(p == parent->p_child && p->p_sib == NULL && p->p_prevSib == NULL){
        /* set child of parent to null *
        parent->p_child = NULL;
        /* set parent of p to null *
        parent = NULL;
        return p;
    }
    /* if p is the first child of its parent*
    if(p == parent->p_child){
        
        /* set new first child to be the old second Child *
        parent->p_child = p->p_sib;
        
        /* change the old head pointers so that its sib and prevSib are null *
        p->p_sib = NULL;
        p->p_sib->p_prevSib = NULL;
        
        /* set the parent of the removed child to be null *
        parent = NULL;
        
        return p;
    }
    /* check to see if p is the last child by seeing if its previous sibling is null *
    if(p->p_sib == NULL){
        /* remove p by setting its prevSib and parent to be null
        make it so that p's previous sib does not point to p*
        p->p_prevSib->p_sib = NULL;
        p->p_prevSib = NULL;
        parent = NULL;

        

        
        
        return p;
    }
    /* if p is a child but not the first child *
    if (p->p_prevSib != NULL && p->p_sib != NULL){
        /* set it so that p's previous sibling and p's next sibling point to eachother and not p *
        p->p_sib->p_prevSib = p->p_prevSib;
        p->p_prevSib->p_sib = p->p_sib;
        
        /* set the parent, and both siblings of p to null *
        parent = NULL;
        p->p_sib = NULL;
        p->p_prevSib = NULL;
        
        return p;
    }
    return NULL;*/


if (p -> p_prnt == NULL) {
        return (NULL);
    }
    
    else {
        /*parent exists and one child*/
        if((p->p_sib == NULL) && (p -> p_prevSib == NULL)){
            /*the pointer to the parent is now null*/
            p -> p_prnt = NULL;
            /*the pointer from the parent to the child is now null*/
            p -> p_prnt -> p_child = NULL;
            /*there should now be no more children*/
            return p;
        }
            
        /*parent exists and child has siblings on each side*/
        else if((p->p_sib != NULL) && (p ->p_prevSib != NULL)){
            /*the childs previous sibling pointing at it goes around it to p's next sibling*/
            p -> p_prevSib -> p_sib = p -> p_sib;
            /*now the childs sibling its pointing to goes around it to p's previous sibling*/
            p -> p_sib -> p_prevSib = p -> p_prevSib;
            p -> p_sib = p -> p_prnt = p -> p_prevSib = NULL;
            return p;
        }
        
        /*parent exists and child has siblings but is first child*/
        else if ((p->p_sib != NULL) && ((p -> p_prevSib) == NULL)){
            /*have the parent now pointing at the sibling*/
            p -> p_prnt -> p_child = p -> p_sib;
            /*delete the sibling pointing back toward the child*/
            /*p -> p_sib -> p_sibPrev = NULL; */
            p -> p_sib = p -> p_prnt = p -> p_prevSib = NULL;
            return p;
        }
        
        /*parent exists and child has siblings but is last child*/
        else{
            /*the previous sibling who is pointing at p which is the sibling gets nulled*/
            p -> p_prevSib -> p_sib = NULL;
            p -> p_prnt = p -> p_prevSib= NULL;
            return p;
        }
    }



    
}
