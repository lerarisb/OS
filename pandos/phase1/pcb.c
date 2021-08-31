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

int emptyProcQ(pcb_t *p){

}

inserProcQ(pcb_t *tp){

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