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

}

pcb_t *allocPcb(){

}

initPcbs(){

}

pcb_t *mkEmptyProcQ(){

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

inserProcQ(pcb_t **tp, pcb_t *p){
    //*tp points to tail, *p is the one being inserted
    //must store old tail
    pcb_t *oldTail = *tp;
    //store head for convenience, head is pcb after tail
    pcb_t *head = *tp.p_next;
    //tp points to a tail, want to to point to new tail
    **tp = *p;
    //new tail must point to same head
    *p.next = *head;
    //new tail must point previously to oldtail
    *p.p_prev = *oldTail;


}

pcb_t *removeProcQ(pcb_t **tp){
    

}

pcb_t *outProcQ(pcb_t **tp, pcb_t *p){

}

pcb_t *headProcQ(pcb_t *tp){

}

int emptyChile(pcb_t *p){

}

insertChild(pcb_t *prnt, pcb_t *p){

}

pcb_t *removeChild(pcb_t *p){

}

pcb_t *outChild(pcb_t *p){
    
}
