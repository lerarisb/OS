#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"

HIDDEN pcb_PTR pcbFree_h;



void freePcb(pcb_t *p){
    /*insert element pointed to by p onto pcbFree list
    pcb is no longer in use, being removed from queue

    check to make sure pcb free list isn't null (that there are pcbs)
    if it is null, p becomes the first element in list*/
    if(pcbFree_h == NULL) {
        pcbFree_h = p;
        return;
    }
    /*add to the head of the linked list of pcbs maintained in pcb free*/
    else{
        /*make p the first element
        p points to the old first element*/
        *p->p_next = *pcbFree_h;
        *pcbFree_h = *p;
    }
}

pcb_t *allocPcb(){
    /*allocated a pcb from the free list to the queue of active pcbs

    check to make sure there is a pcb to remove from free list*/
    if(pcbFree_h == NULL){
        return NULL;
    }

    /*remove the first element from pcb free and add to queue
    removes any previous values from pcb*/

    pcb_t *temp;
    *temp = *pcbFree_h;

    /*setting the pointer to the free list to the next pcb
    essentially removing the current head on free list*/
    *pcbFree_h = *pcbFree_h->p_next;

    /*all previous values of pcb are set to null*/
    temp->p_child = NULL;
    temp->p_sib = NULL;
    temp->p_prnt = NULL;
    temp->p_next = NULL;
    temp->p_prev = NULL;
    return temp;
}

initPcbs(){
    /*initalize the pcbFree list to conatain all the elements of the static array of MAXPROC pcbs
    only called once during data initalization*/
    static pcb_t pcbInit[MAXPROC];
    pcbFree_h = NULL;
    int i = 0;
    while(i < MAXPROC){
        insertProcQ(&pcbFree_h, &pcbInit[i]);
        i++;
    }
}

pcb_t *mkEmptyProcQ(){
    return NULL;
}


int emptyProcQ(pcb_t *tp){
    /*tp points to tail of queue, if queue is empty then return true*/
    return (tp == NULL);


}
/*inserting a pcb into the process queue
given the tail pointer of the process queue and the pcb we want to insert*/
void insertProcQ(pcb_t **tp, pcb_t *p){
    /* *tp points to tail , *p is the one being inserted
    must store old tail*/
    pcb_t *oldTail = *tp;
    /*store head for convenience, head is pcb after tail*/
    pcb_t *head;
    pcb_t *tail;
    tail = *tp;
    tail->p_next = head;
    /*tp points to a tail, want to to point to new tail*/
    (*tp) = p;
    /*new tail must point to same head*/
    *p->p_next = *head;
    /*new tail must point previously to oldtail*/
    *p->p_prev = *oldTail;


}
/*removes the process from the head*/
pcb_t *removeProcQ(pcb_t **tp){
    /*if there are no processes in the queue, there are none to remove*/
    if (emptyProcQ(*tp)){
        return NULL;
    }
    else{
        /*storing oldhead*/
        pcb_t *oldhead;
        pcb_t *tail = *tp;
        tail->p_next = oldhead;
        
        /*storing newhead*/
        pcb_t *newhead = oldhead->p_next;

        /*remove oldhead by setting tail pointer p_next to the next pointer
        tail pointer next is what the old head pointed to
        new head previous is tail*/

        tail->p_next = newhead;
        tail->p_next->p_prev = tail;
    }
    
    
}

/*removes a specified pcb (p) from the queue pointed to by a tail pointer (tp)*/
pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
    /*if the queue has no processes, there are none to be removed*/
    if (emptyProcQ(*tp)){
        return NULL;
}
    else{
        /*store a current node (starting with the head) before we traverse through the queue*/
        pcb_t *currentPcb;
        currentPcb = *tp;
        currentPcb = currentPcb->p_next;

/*if the tail pointer is the pcb you want to remove*/
        if (*tp == p){
            /*remove because desired case
            store old tail*/
            pcb_t *oldTail = *tp;

            /*old tail prev becomes new tail*/
           (*tp)=  oldTail->p_prev;

            /*new tail next points to what old tail did*/
            pcb_t *newtail = *tp;
            newtail->p_next = oldTail->p_next;

            /*head previous points to new tail*/
            newtail->p_next->p_prev = tp;
        }

        /*we continue to traverse until we hit the tail pointer
        if you have not hit the tail pointer but the current pcb is not what you want to remove, keep traversing*/
        while (currentPcb != *tp){
            /*for each current pcb, check to see if it is the desired pcb
            if it is, remove it*/
            if (currentPcb != p){
                /*store the one before the current pcb*/
                pcb_t *prevToRemovalPCB = currentPcb->p_prev;
                /*store the pcb after the current pcb*/
                pcb_t *afterToRemovalPCB = currentPcb->p_next;
                
                /*set the next of previous pcb to the next pcb*/
                *prevToRemovalPCB->p_next = *afterToRemovalPCB;

                /*set the previous of the next pcb to be the previous pcb*/
                *afterToRemovalPCB->p_prev = *prevToRemovalPCB;

            }
            /*if current pcb not what searching for, go to next pcb*/
            
            else{
                currentPcb = currentPcb -> p_next;
            }
            }
            return NULL;
        }
    }

/*return a pointer to first pcb from process queue whose tail is pointed to by a given tail pointer*/
pcb_t *headProcQ(pcb_t *tp){
    /*checks to see if there are processes in the process queue*/
    if (emptyProcQ(tp))
        return (NULL);
    /*returns head of process queue*/
    pcb_t *head = tp->p_next;
    return head; 

}


/*determines if a pcb has any children*/
int emptyChild(pcb_t *p){
    return (p->p_child == NULL);
}

/*adds a child to a pcb that is in process queue*/
void insertChild(pcb_t *prnt, pcb_t *p){
    /*check to see if prnt already has a child*/
    if (emptyChild(prnt)){

    /*if parent does not have a child, add the pcb as one*/
        prnt->p_child = p;
        p->p_prnt = prnt;
        p->p_sib = NULL;
    }
        
    else{
        /*if parent has a child, add pcb as child and move existing children*/
        pcb_t *currentChild =  prnt->p_child;

    /*set new pcb as child*/
        prnt->p_child = p;
        /*set new child to have parent*/
        p->p_prnt = prnt;
        /*recently added pcb must be a sbiling to the existing child*/
        prnt->p_child->p_sib = currentChild;

    }
}
/*remove the head child*/
pcb_t *removeChild(pcb_t *p){
    /*test to see if p has child
    if p has no child, return null*/
    if (emptyChild(p)){
        return NULL;
    }
    
    /*test to see if p has more than one child*/
    pcb_t *currentChild;
    currentChild = p;
    currentChild = currentChild -> p_child;
    if (currentChild->p_sib != NULL){
        /*there is more than one child
        set the second child's parent to *p*/
        currentChild->p_sib->p_prnt = p;
        /*set the child of the parent to be the old sibling*/
        p->p_child = currentChild->p_sib;

        return currentChild;
    }
    /*otherwise p has exactly one child*/
    currentChild = p->p_child;
    /*take old child and set its parent to null*/
    p->p_child->p_prnt = NULL; 
    /*take parent and set child to null*/
    p->p_child = NULL;
    return currentChild;
}

/*removes a specific child*/
pcb_t *outChild(pcb_t *p){
    /*check to see if p has parent*/
    if (p->p_prnt == NULL){
        return NULL;
    }
    /*remove the child*/
    removeChild(p);
}
