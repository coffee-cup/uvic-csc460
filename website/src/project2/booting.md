# RTOS Booting

Since our RTOS's kernel manages all user level processes, it must start before the user level program. We accomplish this by having the C `int main` function inside the kernel. When the kernel has initialized and started, it starts the user program by calling an external `setup` function.

## CRT (C RunTime)

When `avr-gcc` compiles the RTOS C code, it adds the C runtime. The function of this runtime is to

- Initialize memory
- Setup interrupt vector table and interrupt trampolines
- Call the `int main` function

This can be observed by viewing the disassembly of our RTOS. Most of the disassembly is our compiled operating system, but some of it is from the C runtime. The following initializes the interrupt trampolines,

```
000000e4 <__trampolines_start>:
      e4:	0c 94 8c 04 	jmp	0x918	; 0x918 <Kernel_Request_MsgASend>
      e8:	0c 94 bc 00 	jmp	0x178	; 0x178 <Kernel_Request_None>
      ec:	0c 94 31 05 	jmp	0xa62	; 0xa62 <Kernel_Request_MsgRecv>
      f0:	0c 94 b4 06 	jmp	0xd68	; 0xd68 <Kernel_Request_Create>
      f4:	0c 94 73 07 	jmp	0xee6	; 0xee6 <Pong>
      f8:	0c 94 2c 07 	jmp	0xe58	; 0xe58 <Task_Terminate>
      fc:	0c 94 8a 05 	jmp	0xb14	; 0xb14 <Kernel_Request_MsgSend>
     100:	0c 94 2e 06 	jmp	0xc5c	; 0xc5c <Kernel_Request_Terminate>
     104:	0c 94 73 06 	jmp	0xce6	; 0xce6 <Kernel_Request_Next>
     108:	0c 94 d8 00 	jmp	0x1b0	; 0x1b0 <Kernel_Request_GetArg>
     10c:	0c 94 6c 07 	jmp	0xed8	; 0xed8 <Ping>
     110:	0c 94 e2 04 	jmp	0x9c4	; 0x9c4 <Kernel_Request_MsgRply>
     114:	0c 94 b7 00 	jmp	0x16e	; 0x16e <Kernel_idle>
     118:	0c 94 d4 06 	jmp	0xda8	; 0xda8 <setup>
     11c:	0c 94 bd 00 	jmp	0x17a	; 0x17a <Kernel_Request_GetNow>
     120:	0c 94 c8 00 	jmp	0x190	; 0x190 <Kernel_Request_GetPid>
     124:	0c 94 40 04 	jmp	0x880	; 0x880 <Kernel_Request_Timer>
```

We can also see where the `main` function is called after everything has been setup

```
0000015c <.do_clear_bss_start>:
     15c:	a5 38       	cpi	r26, 0x85	; 133
     15e:	b2 07       	cpc	r27, r18
     160:	e1 f7       	brne	.-8      	; 0x15a <.do_clear_bss_loop>
     162:	0e 94 7e 08 	call	0x10fc	; 0x10fc <main>
     166:	0c 94 eb 09 	jmp	0x13d6	; 0x13d6 <exit>
```

The `main` function of our operating system simply initializes and starts the kernel

```c
int main(void) {
    Kernel_Init();
    Kernel_Start();

    /* Control should never reach this point */
    OS_Abort(FAILED_START);

    return -1;
}
```

`Kernel_Init` initializes all processes, messages, process queues, and message queues. Initializing the memory before starting the kernel allows us to never use `malloc` or dynamic memory. In `Kernel_Init` the system clock is configured to generate an interrupt every 10 ms. During initialization we also create a system task which is the user level program `setup` function.

```c
// Add the setup system task
KERNEL_REQUEST_PARAMS info = {
    .request = CREATE,
    .priority = SYSTEM,
    .code = setup,
    .arg = 0
};

request_info = &info;
Kernel_Task_Create();
```

After the kernel has initialized, we start the operating system by entering the kernel for the first time. The first thing we do before entering the infinite kernel loop is to `Dispatch` and find a task to run. Since the only task available is the user defined `setup` task, this is where the user program is started.

In our [future works](#custom-c-runtime) section we explain that an optimization we could make is to include a custom C runtime.

## Memory Layout

No dynamic memory allocation is used so the memory consists entirely of the user code, global variables, and the stack. The kernel can be thought of as its own _process_, but it is really just global variables and not the same as a user process. The kernel variables such as the process array, messages array, kernel stack pointer, system clock, and process queues are created as global variables on the top of the stack.

Each process is has `WORKSPACE` amount of bytes in its stack. This value is currently 256 bytes. When initializing each process we zero 256 bytes clearing out any data in the that processes stack. We then store the address to the `Task_Terminate` function at the bottom of the stack. This prevents stack underrun when a user defined stack does not explicitly call `Task_Terminate` when it has completed. We then place the address to the function which should run as the task. The 16-bit address to the functions must be added to the bottom of the stack in reverse byte order because the return (`ret` and `reti`) instructions pop addresses off in big endian format.

```c
void Kernel_Task_Create_At(PD *p, taskfuncptr f) {
    uint8_t *sp = &(p->workSpace[WORKSPACE - 1]);

    //Clear the contents of the workspace
    ZeroMemory(p->workSpace, WORKSPACE);

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
    p->state = READY;
}
```
