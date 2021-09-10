#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"

semd_t *semdFree_h; 
semd_t *semd_h; 


pcb_t *headBlocked(int *semAdd){
    semd_t *temp;
    temp = search(semAdd); 
    if(temp == NULL || emptyProcQ(temp->s_next->s_procQ)){
        return NULL;
    }else{
        return headProcQ(temp->s_next->s_procQ);
    }
}

void initASL(){

}

semd_t* semAlloc(){

}

int insertBlocked(int *semAdd, pcb_t *p){

}

pcb_t *removeBlocked(int *semAdd){

}

pcb_t *outBlocked(pcb_t *p){

}

void freeSemd(semd_t *semd) {

}


semd_t *search(int *semAdd){

}
