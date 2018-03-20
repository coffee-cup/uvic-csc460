# Context Switching

Context switching is the process of storing the state of the hardware (i.e.: context) of a task and restoring the state (context) of another. When context switching the stack pointer initially points to a position within one task's workspace and is changed to point to another task's workspace. As from the task's perspective, they do not know when this switching is going to happen; in general context switching can occur in the middle of any line of code, which is to say switching can occur at the beginning of any AVR instruction. In order to save a task's context, we need to know what exactly a context is. In the case of the ATmega2560 and our RTOS, a context is

- general purpose registers r0 through r31,
- the status register (located at `0x3F`),
- the extended indirect register (located at `0x3C`), and
- the stack pointer (located at `0x3D -> 0x3E`)

![Context Switching _from [freertos.org](https://www.freertos.org/implementation/a00006.html)_](https://www.freertos.org/implementation/ExeContext.gif)

The context switching code is implemented in assembly and called from our kernel. Saving the context simply involves pushing all of the registers onto the current task's stack.

```assembly
.macro SAVECTX
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r22
    push r23
    push r24
    push r25
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31
    in   r31, EIND
    push r31
    in   r31, SREG
    push r31
.endm
```

Restoring a context is simply the reversing of saving a context. Instead of `push`ing, we `pop`, and instead of reading (`in`), we write (`out`).

When entering the kernel we need to

- disable interrupts,
- save the user level task's context,
- save the hardware stack pointer as the global variable `CurrentSp`,
- set the hardware stack pointer to the kernel's stack pointer from the global variable `KernelSp`, and
- restore the kernel's context.

This process can be seen in the following assembly code. Note that we disable interrupts in the `Kernel_Request` C function.

```assembly
Enter_Kernel:

    ; This is the "bottom" half of Context Switching. We are still executing in
    ; Cp's context.
    SAVECTX

    ; Now, we have saved the Cp's context.
    ; Save the current H/W stack pointer into CurrentSp.
    in   r30, SPL
    in   r31, SPH
    sts  CurrentSp, r30
    sts  CurrentSp+1, r31

    ; We are now ready to restore kernel's context, i.e.,
    ; switching the H/W stack pointer back to KernelSp.
    lds  r30, KernelSp
    lds  r31, KernelSp+1
    out  SPL, r30
    out  SPH, r31

    ; We are now executing in kernel's stack.
    RESTORECTX

    ; We are ready to return to the caller of Exit_Kernel().
    ; Note: We should NOT re-enable interrupts while kernel is running.
    ;       Therefore, we use "ret", and not "reti".
    ret
```

When entering the kernel we want interrupts to be disabled, which is why we call `ret` instead of `reti`.

Exiting the kernel is a similar process. We need to

- save the kernel's context,
- save the hardware stack pointer into the global variable `KernelSp`,
- load the tasks stack pointer from the variable `CurrentSp` into the hardware stack pointer, and
- restore the task's context

This process can be seen in the following assembly function,

```assembly
Exit_Kernel:

    ; This is the "top" half of Exit_Kernel(), called only by the kernel.
    ; Assume I = 0, i.e., all interrupts are disabled.
    SAVECTX

    ; Now, we have saved the kernel's context.
    ; Save the current H/W stack pointer into KernelSp.
    in   r30, SPL
    in   r31, SPH
    sts  KernelSp, r30
    sts  KernelSp+1, r31 ; Now, KernelP->sp = the current H/W stack pointer

    ; We are now ready to restore Cp's context, i.e.,
    ; switching the H/W stack pointer to CurrentSp.
    lds  r30, CurrentSp
    lds  r31, CurrentSp+1
    out  SPL, r30
    out  SPH, r31 ; Now, H/W stack pointer = CurrentP->sp

    ; We are now executing in Cp's stack.
    ; Note: at the bottom of the Cp's context is its return address.
    RESTORECTX
    reti         ; Leaving kernel: re-enable all global interrupts
```

We want interrupts to be enabled when exiting the kernel. This is why we call `reti` instead of `ret`. Importantly, the process of exiting the kernel is the same if we enter the kernel from an external interrupt or from a user kernel request. For both cases we want interrupts to be enabled, so calling `reti` is not a problem.

## 17-bit addressing problem

The original context switching code provided to us was designed for the ATMega1280. [The report by Mike Krazanowski](https://connex.csc.uvic.ca/access/content/group/a665cfb0-e5ea-4601-adab-bdbea57c8ab7/Projects%20Related/Project%202/RTOS%20Sample%20Code/krazanowski_a2.pdf), explains the 17-bit addressing problem very well and greatly helped us in developing our context switching. The ATMega1280 used 16 bit addressing as there was only 128K of memory available. Since the ATMega2560 has 256K available, an extra register is needed to address all entire memory space. This extra register is the extended indirect register (`EIND`). See [Memory Layout](#memory-layout) to see the initialization of the EIND byte, which can be initially written as zero because the compiler uses trampolines to access higher memory addresses. When saving the stack pointers, 3 bytes needs to be popped and restored as opposed to 2, as is with the `ret` and `reti` assembly instructions - both pop 3 bytes from the stack. This can be done by simply modifying the saving and restoring context assembly code to account for the extra byte.

![17-bit addressing problem (Mike Krazanowski)](https://i.imgur.com/2ozBAyA.png)
