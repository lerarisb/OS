#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"

HIDDEN pcb_PTR pcbFree_h;

/*insert element pointed to by p onto pcbFree list
    pcb is no longer in use, being removed from queue*/ 
void freePcb(pcb_PTR p){
    insertProcQ(&pcbFree_h, p);
}

/*allocated a pcb from the free list to the queue of active pcbs */
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

/*initalize the pcbFree list to conatain all the elements of the static array of 
MAXPROC pcbs only called once during data initalization*/
void initPcbs(){
    static pcb_t pcbInit[MAXPROC];
    pcbFree_h = NULL;
    int i = 0;
    while(i < MAXPROC){
        insertProcQ(&pcbFree_h, &pcbInit[i]);
        i++;
    }
}

/* return pointer to the tail of an empty process queue */
pcb_PTR mkEmptyProcQ(){
    return NULL;
}

/* return true if the tail pointed to by tp is empty false if otherwise */
int emptyProcQ(pcb_PTR tp){
    return (tp == NULL);
}

/*inserting a pcb into the process queue
given the tail pointer of the process queue and the pcb we want to insert*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p){
    /* checks if it is empty 
    if empty make it circular */
    if(emptyProcQ((*tp))){
        p->p_next = p;
    	p->p_prev = p;
    }
    /*checks if there is one singular node 
    if only one node, then insert a node at the tail
    inserted node points to the one node that was already there
    node that was already there points to inserted node */
   /* else if((*tp)->p_next == (*tp)){
        p->p_next = *tp;
        p->p_prev = *tp;
        (*tp)->p_next = p;
        (*tp)->p_prev = p;
    }*/
    /* if there are more than one nodes */
    else {


        /* store current tail */
        pcb_PTR currentTail = *tp;
        /* set new node next to currentTail next */ 
        p->p_next = currentTail->p_next;
        /* set currentTail next to new node */
        currentTail->p_next = p;
        /* set new node prev to current tail */
        p->p_prev = currentTail;
        /* set head of queue prev to new tail */
        p->p_next->p_prev = p;
    
        
    }
    /* set tail pointer to new node */
    *tp = p;

}

/*removes the node from the head*/
pcb_PTR removeProcQ(pcb_PTR *tp){
    /*if there are no nodes in the queue, there are none to remove*/
    if (emptyProcQ(*tp)){
        return NULL;
    }
    return (outProcQ(tp, (*tp)->p_next));
}

/*removes a specified pcb (p) from the queue pointed to by a tail pointer (tp)*/
pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p){
  pcb_PTR head = (*tp)->p_next;
   if (emptyProcQ(*tp)){
            return NULL;
   }
    while (p != head && head != *tp){
        head = head->p_next;
    }
    if (head == p){
        head->p_next->p_prev = head->p_prev;
        head->p_prev->p_next = head->p_next;
        if(head == *tp){
            *tp = head->p_prev;
        }
        if (p->p_next == p){
            *tp = NULL;
        }
        p->p_next = NULL;
        p->p_prev = NULL;
        return p;
    }
    return NULL;
}

/*return a pointer to first pcb from process queue whose tail is pointed to by a given tail pointer*/
pcb_PTR headProcQ(pcb_PTR tp){
    /*checks to see if process queue is empty*/
    if (emptyProcQ(tp)){
        return NULL;
    }
    /*if not empty returns head of process queue*/
    return (tp->p_next); 
}

/*determines if a pcb has any children true if none false if there is one or more*/
int emptyChild(pcb_PTR p){
    return (p->p_child == NULL);
}

/*adds a child to a pcb that is in process queue*/
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
        /*set new pcb as child*/
        prnt->p_child = p;
        /* set old first child prevSib to new child */
        firstChild->p_prevSib = p;
        /*set new child to have parent*/
        p->p_prnt = prnt;
        /*recently added pcb must be a sbiling to the existing child*/
        p->p_sib = firstChild;
        /* set new child prev sib to null */
        p->p_prevSib = NULL;
    }
}

/*remove the head child*/
pcb_PTR removeChild(pcb_PTR p){
    /* store first child for future use */
    pcb_PTR currentChild = p->p_child;
    /*test to see if p has child
    if p has no child, return null*/
    if (emptyChild(p)){
        return NULL;
    }
    /* if there is only one child */
    else if (currentChild->p_sib == NULL){
        /*take only child and set its parent to null*/
        currentChild->p_prnt = NULL; 
        /*take parent and set child to null*/
        p->p_child = NULL;
        return currentChild;
    }
    /*if p has more than one child*/
    else{
        /* set new child of p to the current childs sib */
        p->p_child = currentChild->p_sib;
        /* set parent and sib of current child to null */
        currentChild->p_prnt = NULL;
        currentChild->p_sib = NULL;
        /* set prevSib of new head child to null */
        p->p_child->p_prevSib = NULL;
        return currentChild;
    }   
}

/*removes a specific child*/
pcb_PTR outChild(pcb_PTR p){
   /*store parent of p for further use */
    pcb_PTR parent = p->p_prnt;
    if (p == NULL){
        return NULL;
    }
    /*check to see if p has parent*/
    if (parent == NULL){
        return NULL;
    }
    /* if the only child of its parent */
    if(p == parent->p_child && p->p_sib == NULL && p->p_prevSib == NULL){
        /* set child of parent to null */
        parent->p_child = NULL;
        /* set parent of p to null */
        parent = NULL;
        return p;
    }
    /* if first child is p */
    if(p == parent->p_child){
        /* set new head child */
        parent->p_child = p->p_sib;
        /* change old head child sib and prevSib to null */
        p->p_sib = NULL;
        p->p_sib->p_prevSib = NULL;
        /* set removed child parent to null */
        parent = NULL;
        return p;
    }
    /* check last child */ 
    if(p->p_sib == NULL){
        /* if last child set prevSib to null, the prevSib sib to null and parent
        to null */
        p->p_prevSib->p_sib = NULL;
        p->p_prevSib = NULL;
        parent = NULL;
        return p;
    }
    /* if not first child */
    if (p->p_prevSib != NULL && p->p_sib != NULL){
        /* set the new sib and prevSib of the two siblings on both sides of p */
        p->p_sib->p_prevSib = p->p_prevSib;
        p->p_prevSib->p_sib = p->p_sib;
        /* set the parent, and both siblings of p to null */
        parent = NULL;
        p->p_sib = NULL;
        p->p_prevSib = NULL;
        return p;
    }
    return NULL;
}


