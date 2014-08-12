/*
The MIT License (MIT)

Copyright (c) 2014 Xavier Hardy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef INTERPRETER_H_INCLUDED
#define INTERPRETER_H_INCLUDED
#define MEMORY_SIZE	65555 //32bits 4294967295
#define MEMORY_START	65555
const int MAX_CODE_SIZE = 100;
unsigned char memory[MEMORY_SIZE];

typedef struct instruction {
    unsigned int op;
    unsigned int *dest; //Pointer to the destination register
    int deststr; //str, understand: start, where is starting the "subregister", is a low one (0) or high one
    unsigned int *reg;
    int regstr;
    unsigned int *src1;
    int src1str;
    unsigned int *src2;
    int src2str;
    unsigned int disp; //displacement
    unsigned int scale; //scale (in SIB)
    unsigned int twoSrc; //does it have two sources
    unsigned int length; //length in byte
    unsigned int imd; //immediate value in grp1 operations
    void* label;
} INSTR;

typedef struct operation {
    unsigned char *name;
    void *op1label; //if there is two modes
    void *op2label;
    void *initlabel;
} OP;

typedef struct group {
    void *label[8];
} GRP;

//This table contains all the non-extended-opcode operations, if an instruction is not implemented, it displays an error and stops the program
OP* dispatch;
;
unsigned int registers[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

char *regnames[]={"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI", "EIP", "EFLAGS", "CS", "SS", "DS", "ES", "FS", "GS"};
//, "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH", "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
unsigned int zero = 0; //This is used to replace the index register (in SIB)
//EAX ECX EDX EBX ESP EBP ESI EDI AX AH AL BX BH BL CX CH CL DX DH DL SI  DI BP SP EIP EFLAGS

/*%eax accumulator (for adding, multiplying, etc.)
%ebx base (address of array in memory)
%ecx count (of loop iterations)
%edx data (e.g., second operand for binary operations)
%esi source index (for string copy or array access)
%edi destination index (for string copy or array access)
%ebp base pointer (base of current stack frame)
%esp stack pointer (top of stack)
%eip instruction pointer (program counter)
%eflags flags (condition codes and other things)
*/

#endif // INTERPRETER_H_INCLUDED
