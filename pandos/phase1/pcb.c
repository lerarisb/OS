#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"

typedef struct pcb_t{
    struct pcb_t        *p_next, 
                        *p_prev,

                        *p_prnt,
                        *p_child,
                        *p_sib;

    state_t             p_s;
    cpu_t               p_time;
    int                 *p_semAdd;

//support layer info page 8 not included because of error

} pcb_t;

void freePcb(pcb_t *p){
    if(pcbFree_h == NULL) {
        pcbFree_h = p;
        return;
    }
    else{
        p->p_next = pcbFree_h;
        pcbFree_h = p;
    }
}

pcb_t *allocPcb(){
    if(pcbFree_h == NULL){
        return NULL;
    }
    pcb_t temp;
    temp = pcbFree_h;
    pcbFree_h = pcbFree_h->p_next;
    temp->p_child = NULL;
    temp->p_sib = NULL;
    temp->p_prevsib = NULL;
    temp->p_prnt = NULL;
    temp->p_next = NULL;
    temp->p_prev = NULL;
    return temp;
}

initPcbs(){
    static pcb_t pcbInit[MAXPROC];
    pcbFree_h = NULL;
    int i = 0;
    while(i < MAXPROC){
        insertProcQ(&pcbFree_h, &pcbInit[i]);
        i++;
}

pcb_t *mkEmptyProcQ(){
    return NULL;
}


int emptyProcQ(pcb_t *tp){
    //tp points to tail of queue, if queue is empty then return true
    if (*tp == null){
        return TRUE
    }
    else{
        return FALSE
    }

}

insertProcQ(pcb_t **tp, pcb_t *p){
    //*tp points to tail, *p is the one being inserted
    //must store old tail
    pcb_t *oldTail = *tp;
    //store head for convenience, head is pcb after tail
    pcb_t *head = *tp->p_next;
    //tp points to a tail, want to to point to new tail
    **tp = *p;
    //new tail must point to same head
    *p->p_next = *head;
    //new tail must point previously to oldtail
    *p->p_prev = *oldTail;


}

pcb_t *removeProcQ(pcb_t **tp){
    if (emptyProcQ(*tp)){
        return NULL;
    }
    else{
        //storing oldhead
        pcb_t *oldhead = *tp->p_next;
        
        //storing newhead
        pcb_t *newhead = *oldhead->p_next;

        //remove oldhead by setting tail pointer p_next to the next pointer
        //tail pointer next is what the old head pointed to
        //new head previous is tail

        *tp->p_next = *newhead
        *tp->p_next->p_prev = *tp;
    }
    

}

pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
    if (emptyProcQ(*tp)){
        return Null;
}
    else{
        //store a current node
        pcb_t *currentPcb = *tp->p_next;

        if (*tp = *p){
            //remove because desired case
            //store old tail
            pcb_t *oldTail = *tp;

            //old tail prev becomes new tail
            *tp =  *oldTail->p_prev;

            //new tail next points to what old tail did
            *tp->p_next = *oldTail->p_next;

            //head previous points to new tail
            *tp->p_next->p_prev = *tp;
        }

        //the test is, if the current pcb is node the last pcb
        //basically, checking to make sure you haven't gone through the whole queue
        while (*currentPCb != *tp){
            //for each current pcb, check to see if it is the desired pcb
            //if it is, remove it
            if (*currentPcb != *p){
                //remove it
            }
            //if not, go to next pcb
            else{
                *currentPcb = currentPcb -> p_next;
            }
            }
        }
    }
}

pcb_t *headProcQ(pcb_t *tp){
    if (emptyProcQ(*tp))
        return (null);
    pcb_t *head = *tp->p_next;
    return *head; 

}

int emptyChile(pcb_t *p){
    if (*p.p_child == NULL)
        return TRUE
    return FALSE
}

insertChild(pcb_t *prnt, pcb_t *p){
    //check to see if prnt already has a child
    if (emptyChile(*prnt))
        *prnt.p->p_child = *p;
    *p.p->p_prnt = *prnt;
    //prnt already has a child
    else{
        pcb_t *currentChild = *prnt->p_child;
        //set child to its parent
        *prnt->p_child = *p;
        //set child to its sibling (old child)
        *prnt->p.child->p_sib = *currentChild;

    }
}

pcb_t *removeChild(pcb_t *p){
    //test to see if p has child
    if (emptyChile(*p))
        return NULL;
    }
    //test to see if p has more than one child
    pcb_t *currentChild = *p->p_child;
    if (*currentChild->p_sib != NULL){
        //there is more than one child
        //set the second child's parent to *p
        *currentChild->p_sib->p_parent = *p;
        //set the child of the parent to be the old sibling
        *p->p_child = *currentChild->p_sib;
        return *currentChild;
    }
    //otherwise p has exactly one child
    *currentChild = *p->p_child;
    //take old child and set its parent to null
    *p->p_child->p_parent = NULL; 
    //take parent and set child to null
    *p->p_child = NULL;
    return*currentChild;

pcb_t *outChild(pcb_t *p){
    //check to see if p has parent
    if (*p->p_parent == NULL){
        return null;
    }
    //p has a parent
    pcb_t *parent = *p->p_parent;
    //remove the child of p
    removeChild(*parent);
}
