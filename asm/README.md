### How to Use Inline Assembly Language in C Code

https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html

The `asm` keyword allows you to embed assembler instructions within C code. 

GCC provides two forms of inline asm statements. 

- basic asm statement

A basic asm statement is one with no operands (see Basic Asm)

to include assembly language at top level you must use basic asm.

- extended asm statement

an extended asm statement (see Extended Asm) includes one or more operands. 

The extended form is preferred for mixing C and assembly language within a function

#### Basic Asm

```
asm asm-qualifiers ( AssemblerInstructions )

```

asm-qualifiers:

- volatile: The optional volatile qualifier has no effect. All basic asm blocks are implicitly volatile.

- inline: 

AssemblerInstructions:

This is a literal string that specifies the assembler code.

GCC does not parse the assembler instructions themselves and does not know what they mean or even whether they are valid assembler input.

#### Extended Asm


With extended asm you can read and write C variables from assembler and perform jumps from assembler code to C labels.

Extended asm syntax uses colons (':') to delimit the operand parameters after the assembler template:

```
asm asm-qualifiers ( AssemblerTemplate 
                 : OutputOperands 
                 [ : InputOperands
                 [ : Clobbers ] ])


```

asm-qualifiers:

- volatile
- inline
- goto

AssemblerTemplate

OutputOperands

InputOperands:  

Input operands make values from C variables and expressions available to the assembly code.

```
[ [asmSymbolicName] ] constraint (cexpression)

```



Clobbers




```c

static void outb(uint16_t port, uint8_t value) {
        asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

```


```
asm asm-qualifiers ( AssemblerTemplate 
                      : OutputOperands
                      : InputOperands
                      : Clobbers
                      : GotoLabels)

```


