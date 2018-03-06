#include "kernel.h"


void Kernel_Create_Task_At(PD *p, voidfuncptr f) {
    uint8_t *sp = &(p->workSpace[WORKSPACE - 1]);

    //Clear the contents of the workspace
    ZeroMemory(p->workSpace, WORKSPACE);

    //Notice that we are placing the address (16-bit) of the functions
    //onto the stack in reverse byte order (least significant first, followed
    //by most significant).  This is because the "return" assembly instructions
    //(rtn and rti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
    //second), even though the AT90 is LITTLE ENDIAN machine.

    //Store terminate at the bottom of stack to protect against stack underrun.
    *sp-- = LOW_BYTE(Task_Terminate);
    *sp-- = HIGH_BYTE(Task_Terminate);
    *sp-- = LOW_BYTE(0);

    //Place return address of function at bottom of stack
    *sp-- = LOW_BYTE(f);
    *sp-- = HIGH_BYTE(f);
    *sp-- = LOW_BYTE(0);
//Place stack pointer at top of stack
    sp = sp - 34;

    p->sp = sp;      /* stack pointer into the "workSpace" */
    p->code = f;     /* function to be executed as a task */
    p->request = NONE;
    p->state = READY;
}

static void Kernel_Create_Task(voidfuncptr f) {
    int x;

    if (Tasks == MAXTHREAD) return;  /* Too many task! */

    /* find a DEAD PD that we can use  */
    for (x = 0; x < MAXTHREAD; x++) {
        if (Process[x].state == DEAD) break;
    }

    ++Tasks;
    Kernel_Create_Task_At( &(Process[x]), f );
}

/**
 * This internal kernel function is a part of the "scheduler". It chooses the
 * next task to run, i.e., Cp.
 */
static void Dispatch() {
    /* find the next READY task
     * Note: if there is no READY task, then this will loop forever!.
     */
    while(Process[NextP].state != READY) {
        NextP = (NextP + 1) % MAXTHREAD;
    }

    Cp = &(Process[NextP]);
    CurrentSp = Cp->sp;
    Cp->state = RUNNING;

    NextP = (NextP + 1) % MAXTHREAD;
}

static void Next_Kernel_Request() {
    Dispatch();  /* select a new task to run */

    while(1) {
        Cp->request = NONE; /* clear its request */

        /* activate this newly selected task */
        CurrentSp = Cp->sp;
        Exit_Kernel();    /* or CSwitch() */

        /* if this task makes a system call, it will return to here! */

        /* save the Cp's stack pointer */
        Cp->sp = CurrentSp;

        switch(Cp->request){
        case CREATE:
            Kernel_Create_Task( Cp->code );
            break;
        case NEXT:
        case NONE:
            /* NONE could be caused by a timer interrupt */
            Cp->state = READY;
            Dispatch();
            break;
        case TERMINATE:
            /* deallocate all resources used by this task */
            Cp->state = DEAD;
            Dispatch();
            break;
        default:
            /* Houston! we have a problem here! */
            break;
        }
    }
}

int main() {

}
