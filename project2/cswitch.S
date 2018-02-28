/*
 * Compile using:
 *   avr-gcc -c -O2 -mmcu=${CPU} -Wa,--gstabs -o switch.o switch.S
 */

/*
  * Note:
  *
  * This code is based on "thread_swtch.S" by Brian S. Dean, and the
  * "os_cpu_a.asm" of uC/OS-II AVR Specific code by Ole Saether. 
  * They are adapted to match our need of a "full-served" kernel model.
  *
  *  Author:  Dr. Mantis Cheng, 2 October 2006
  *
  *  ChangeLog: Modified by Alexander M. Hoole, October 2006.
  *
  *  !!!!!   This code has NEVER been tested.  !!!!!
  *  !!!!!   Use at your own risk  !!!!
  */


/* locations of well-known registers */
SREG  = 0x3F
SPH    = 0x3E
SPL    = 0x3D

/*
  * MACROS
  */
;
; Push all registers and then the status register.
; It is important to keep the order of SAVECTX and RESTORECTX  exactly
; in reverse. Also, when a new process is created, it is important to 
; initialize its "initial" context in the same order as SAVECTX.
;	
.macro	SAVECTX
	push	r0
	push	r1
	push	r2
	push	r3
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	push	r12
	push	r13
	push	r14
	push	r15
	push	r16
	push	r17
	push	r18
	push	r19
	push	r20
	push	r21
	push	r22
	push	r23
	push	r24
	push	r25
	push	r26
	push	r27
	push	r28
	push	r29
	push	r30
	push	r31
	in	r16, SREG
	push	r16
.endm
;
; Pop all registers and the status registers
;
.macro	RESTORECTX
	pop	r16
	out	SREG,r16
	pop	r31
	pop	r30
	pop	r29
	pop	r28
	pop	r27
	pop	r26
	pop	r25
	pop	r24
	pop	r23
	pop	r22
	pop	r21
	pop	r20
	pop	r19
	pop	r18
	pop	r17
	pop	r16
	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	r7
	pop	r6
	pop	r5
	pop	r4
	pop	r3
	pop	r2
	pop	r1
	pop	r0
.endm

        .section .text
        .global CSwitch
        .global Exit_Kernel
        .extern Dispatch
        .extern CurrentP     /* CurrentP->sp is the first entry in CurrentP's descriptor  */
/*
  * The actual CSwitch() code begins here.
  *
  * This function is called by a user task. Upon entry, we are using
  * the CurrentP's stack, on top of which contains the return address 
  * of the call to CSwitch(). There are three phases in this function:
  *   1) save current CurrentP's context, and then its H/W stack pointer;
  *   2) choose a new CurrentP;
  *   3) restore the new CurrentP's context, i.e., restore its H/W stack pointer.
  * 
  * Assumption: This function is executed with interrupts already disabled.
  *
  * Note: AVR devices use LITTLE endian format, i.e., a 16-bit value starts
  * with the lower-order byte first, then the higher-order byte.
  *
  * void CSwitch();
  * void Exit_Kernel(); 
  */
CSwitch:
        /* 
          * Assume I = 0, i.e., all interrupts are disabled.
          */
        SAVECTX
        /* 
          * Now, we have saved the CurrentP's context.
          * Save the current H/W stack pointer into CurrentP->sp.
          */
        lds  r30, CurrentP
        lds  r31, CurrentP+1   /* Z = (R31,R30) contains the "sp" */
        in   r28, SPL
        in   r29, SPH
        st   Z+, r28       /* store (R28=SPL,R29=SPH) indirectly via Z */
        st   Z+, r29       /* Now, CurrentP->sp = the current H/W stack pointer */
       /*
         * Now, we should select a new "CurrentP".
         */
Exit_Kernel:
        call Dispatch
        /*
          * We are now ready to restore a new CurrentP's context, i.e.,
          * switching the H/W stack pointer to this new CurrentP->sp.
          */ 
        lds  r30, CurrentP
        lds  r31, CurrentP+1  /* Z = (R31,R30) contains the new "sp" */
        ld   r28, Z+
        ld   r29, Z+
        out  SPL, r28
        out  SPH, r29   /* Now, H/W stack pointer = CurrentP->sp */
        /*
          * We are now executing in our new CurrentP's stack.
          * Note: at the bottom of the CurrentP's context is its return address.
          */
        RESTORECTX
        ret    /* don't use rti to re-enable interrupts */
/* end of CSwitch() */
