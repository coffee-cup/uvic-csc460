# Future work
For project 3 we will need some mechanisms for generating PWM signals, and using the ATmega2560's analog to digital converters. Work that is to be completed is tracked in our [GitHub issues page](https://github.com/coffee-cup/uvic-csc460/issues). Additionally some features we would like to implement are explained briefly below.

## Custom C Runtime
If we provide our own assembly for a `c` runtime, we could have the booting code call a kernel initialization function rather than '`main()`'. This way, the main function can be reserved for the user level code, helping to further abstract away the os. Currently the instruction that calls main is in the disassembly at offset `0x162`.

```assembly
0000015c <.do_clear_bss_start>:
     15c:	a5 38       	cpi	r26, 0x85	; 133
     15e:	b2 07       	cpc	r27, r18
     160:	e1 f7       	brne	.-8      	; 0x15a <.do_clear_bss_loop>
     162:	0e 94 9d 08 	call	0x113a	; 0x113a <main>
     166:	0c 94 0e 0a 	jmp	0x141c	; 0x141c <exit>
```

## Stack overflow detection

In order to detect if a task has performed some stack overflow by writing beyond the current 256 byte stack size allocation, a stack canary could be utilized. A stack canary is a special value written to 1 or more bytes at the top limit of a stack. If the stack canary is modified, then the application's stack must have written to the top or past the top of its stack (workspace). This can cause problems as data in adjacent workspaces might be changed. Whenever possible, the code should check the stack canary is in tact. As errors of this sort can have unintended consequences.

Another approach is to compare the size of the start of the workspace pointer with the task's current stack pointer.

