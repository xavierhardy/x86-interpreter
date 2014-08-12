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

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"

int pow2(int i){
    int r = 1;
    while(i > 0)
        r *= 2;
    return r;
}

unsigned int getdisp(FILE *file, int i){ //Find the displacement
    unsigned int factor = 1;
    unsigned int disp = 0;
    char x[2];
    while(i > 0){
        fgetc(file);
        disp += factor * strtol(fgets(x,3,file),NULL,16);
        factor *= 256;
        i--;
    }
    return disp;
}

void set32mem(unsigned int i, unsigned int adr){ //Set a 32 bit value in the memory
    int j;
    for(j = 0; j < 4; j++){
        memory[(adr-MEMORY_START)%MEMORY_SIZE] = (unsigned char) i;
        adr--;
        i >>= 8;
    }
}

unsigned int get32mem(unsigned int adr){ //get a 32 bit value from the memory
    int j;
    unsigned int i = 0;
    for(j = 0; j < 4; j++){
        i += memory[(adr-MEMORY_START)%MEMORY_SIZE] << (j*8);
        adr--;
    }
    return i;
}

void set16mem(unsigned short i, unsigned int adr){
    int j;
    for(j = 0; j < 2; j++){
        memory[(adr-MEMORY_START)%MEMORY_SIZE] = (unsigned char) i;
        adr--;
        i >>= 8;
    }
}

unsigned short get16Mem(unsigned int adr){
    int j;
    unsigned short i = 0;
    for(j = 0; j < 2; j++){
        i += memory[(adr-MEMORY_START)%MEMORY_SIZE] << (j*8);
        adr--;
    }
    return i;
}

char *bin(unsigned int x){ //Write under binary form
    static char binval[32];
    unsigned int y = x;
    binval[0] = '\0';

    int z;
    for (z = 31; z >= 0; z--){
        if(y%2)
            binval[z] = '1';
        else
            binval[z] = '0';
        y /= 2;
    }

    return binval;
}

char printb(unsigned int x, char *str){ // Print some information on a value
    printf("%+8X : %s [%s (%u)]\n", x, str, bin(x), x);
}

void initregs(){ //Initialise registers
    registers[0] = strtol ("bf8db144",NULL,16);
    registers[1] = strtol ("88c5cffb",NULL,16);
    registers[2] = strtol ("1",NULL,16);
    registers[3] = strtol ("ae5ff4",NULL,16);
    registers[4] = strtol ("bf8db0bc",NULL,16);
    registers[5] = strtol ("bf8db118",NULL,16);
    registers[6] = strtol ("9a0ca0",NULL,16);
    registers[7] = strtol ("0",NULL,16);
    registers[8] = strtol ("8048354",NULL,16);
    registers[9] = strtol ("246",NULL,16);
    registers[10] = strtol ("73",NULL,16);
    registers[11] = strtol ("7b",NULL,16);
    registers[12] = strtol ("7b",NULL,16);
    registers[13] = strtol ("7b",NULL,16);
    registers[14] = strtol ("0",NULL,16);
    registers[15] = strtol ("33",NULL,16);
}

void printregs(){ //Print the contents of the registers
    unsigned int *regval = &registers[0];
    char **regnam = &regnames[0];
    int i;
    for(i = 0; i < 10; i++){
        printb(*regval, *regnam);
        regval++;
        regnam++;
    }
    //printf("EAX = %u", *regval);
    //printf("EAX = %o", *regval);
}

int main (int argc, char *argv[])
{

    struct instruction code[MAX_CODE_SIZE]; //Our code
    int i = 0;
    initregs();
    unsigned int *EAX = &registers[0]; //Some pointers to registers, it's faster and easier to read
    unsigned int *AL = &registers[0];  
    unsigned int *ESP = &registers[4];
    unsigned int *BX = &registers[3];
    unsigned int *BP = &registers[5];
    unsigned int *SI = &registers[6];
    unsigned int *DI = &registers[7];
    unsigned int *EIP = &registers[8];
    unsigned int *EFLAGS = &registers[9];
    int halt = 0, interrupt = 0;

    //Instructions, the name (or group), two possible instruction labels, the initialization label
    OP dispatch[]={
        {"ADD", &&addGbGb, &&addAbGb, &&initTO},		//000		00	ADD		Eb	Gb
        {"ADD", &&addGvGv, &&addAvGv, &&initTO},		//001		01	ADD		Ev	Gv
        {"ADD", &&addGbGb, &&addGbAb, &&initTO},		//002		02	ADD		Gb	Eb
        {"ADD", &&addGvGv, &&addGvAv, &&initTO},		//003		03	ADD		Gv	Ev
        {"ADD", &&addIbAL, &&label, &&initImmediate},		    //004		04	ADD		AL	Ib
        {"ADD", &&addIvEAX, &&label, &&initImmediate},		    //005		05	ADD		eAX	Iv
        {"PUSH", &&label, &&label, &&initlabel},		//006		06	PUSH	ES
        {"POP", &&label, &&label, &&initlabel},	    	//007		07	POP		ES
        {"OR", &&label, &&label, &&initTO},		//008		08	OR		Eb	Gb
        {"OR", &&label, &&label, &&initTO},		//009		09	OR		Ev	Gv
        {"OR", &&label, &&label, &&initTO},		//010		0A	OR		Gb	Eb
        {"OR", &&label, &&label, &&initTO},		//011		0B	OR		Gv	Ev
        {"OR", &&label, &&label, &&initImmediate},		//012		0C	OR		AL	Ib
        {"OR", &&label, &&label, &&initImmediate},		//013		0D	OR		eAX	Iv
        {"PUSH", &&label, &&label, &&initlabel},		//014		0E	PUSH	CS
        {"--", &&label, &&label, &&initlabel},		//015		0F	--
        {"ADC", &&label, &&label, &&initTO},		//016		10	ADC		Eb	Gb
        {"ADC", &&label, &&label, &&initTO},		//017		11	ADC		Ev	Gv
        {"ADC", &&label, &&label, &&initTO},		//018		12	ADC		Gb	Eb
        {"ADC", &&label, &&label, &&initTO},		//019		13	ADC		Gv	Ev
        {"ADC", &&label, &&label, &&initImmediate},		//020		14	ADC		AL	Ib
        {"ADC", &&label, &&label, &&initImmediate},		//021		15	ADC		eAX	Iv
        {"PUSH", &&label, &&label, &&initlabel},		//022		16	PUSH	SS
        {"POP", &&label, &&label, &&initlabel},		//023		17	POP		SS
        {"SBB", &&label, &&label, &&initTO},		//024		18	SBB		Eb	Gb
        {"SBB", &&label, &&label, &&initTO},		//025		19	SBB		Ev	Gv
        {"SBB", &&label, &&label, &&initTO},		//026		1A	SBB		Gb	Eb
        {"SBB", &&label, &&label, &&initTO},		//027		1B	SBB		Gv	Ev
        {"SBB", &&label, &&label, &&initImmediate},		//028		1C	SBB		AL	Ib
        {"SBB", &&label, &&label, &&initImmediate},		//029		1D	SBB		eAX	Iv
        {"PUSH", &&label, &&label, &&initlabel},		//030		1E	PUSH	DS
        {"POP", &&label, &&label, &&initlabel},		//031		1F	POP		DS
        {"AND", &&label, &&label, &&initTO},		//032		20	AND		Eb	Gb
        {"AND", &&label, &&label, &&initTO},		//033		21	AND		Ev	Gv
        {"AND", &&label, &&label, &&initTO},		//034		22	AND		Gb	Eb
        {"AND", &&label, &&label, &&initTO},		//035		23	AND		Gv	Ev
        {"AND", &&label, &&label, &&initImmediate},		//036		24	AND		AL	Ib
        {"AND", &&label, &&label, &&initImmediate},		//037		25	AND		eAX	Iv
        {"ES:", &&label, &&label, &&initlabel},		//038		26	ES:
        {"DAA", &&label, &&label, &&initlabel},		//039		27	DAA
        {"SUB", &&label, &&label, &&initTO},		//040		28	SUB		Eb	Gb
        {"SUB", &&label, &&label, &&initTO},		//041		29	SUB		Ev	Gv
        {"SUB", &&label, &&label, &&initTO},		//042		2A	SUB		Gb	Eb
        {"SUB", &&label, &&label, &&initTO},		//043		2B	SUB		Gv	Ev
        {"SUB", &&label, &&label, &&initImmediate},		//044		2C	SUB		AL	Ib
        {"SUB", &&label, &&label, &&initImmediate},		//045		2D	SUB		eAX	Iv
        {"CS:", &&label, &&label, &&initlabel},		//046		2E	CS:
        {"DAS", &&label, &&label, &&initlabel},		//047		2F	DAS
        {"XOR", &&label, &&label, &&initTO},		//048		30	XOR		Eb	Gb
        {"XOR", &&label, &&label, &&initTO},		//049		31	XOR		Ev	Gv
        {"XOR", &&label, &&label, &&initTO},		//050		32	XOR		Gb	Eb
        {"XOR", &&label, &&label, &&initTO},		//051		33	XOR		Gv	Ev
        {"XOR", &&xorIbAL, &&label, &&initImmediate},		//052		34	XOR		AL	Ib
        {"XOR", &&label, &&label, &&initImmediate},		//053		35	XOR		eAX	Iv
        {"SS:", &&label, &&label, &&initlabel},		//054		36	SS:
        {"AAA", &&label, &&label, &&initlabel},		//055		37	AAA
        {"CMP", &&label, &&label, &&initTO},		//056		38	CMP		Eb	Gb
        {"CMP", &&label, &&label, &&initTO},		//057		39	CMP		Ev	Gv
        {"CMP", &&label, &&label, &&initTO},		//058		3A	CMP		Gb	Eb
        {"CMP", &&label, &&label, &&initTO},		//059		3B	CMP		Gv	Ev
        {"CMP", &&label, &&label, &&initImmediate},		//060		3C	CMP		AL	Ib
        {"CMP", &&label, &&label, &&initImmediate},		//061		3D	CMP		eAX	Iv
        {"DS:", &&label, &&label, &&initlabel},		//062		3E	DS:
        {"AAS", &&label, &&label, &&initlabel},		//063		3F	AAS
        {"INC",	&&inc, &&initDOO, &&initDOO},		//064		40	INC		eAX
        {"INC", &&inc, &&initDOO, &&initDOO},		//065		41	INC		eCX
        {"INC", &&inc, &&initDOO, &&initDOO},		//066		42	INC		eDX
        {"INC", &&inc, &&initDOO, &&initDOO},		//067		43	INC		eBX
        {"INC", &&inc, &&initDOO, &&initDOO},		//068		44	INC		eSP
        {"INC", &&inc, &&initDOO, &&initDOO},		//069		45	INC		eBP
        {"INC", &&inc, &&initDOO, &&initDOO},		//070		46	INC		eSI
        {"INC", &&inc, &&initDOO, &&initDOO},		//071		47	INC		eDI
        {"DEC", &&dec, &&initDOO, &&initDOO},		//072		48	DEC		eAX
        {"DEC", &&dec, &&initDOO, &&initDOO},		//073		49	DEC		eCX
        {"DEC", &&dec, &&initDOO, &&initDOO},		//074		4A	DEC		eDX
        {"DEC", &&dec, &&initDOO, &&initDOO},		//075		4B	DEC		eBX
        {"DEC", &&dec, &&initDOO, &&initDOO},		//076		4C	DEC		eSP
        {"DEC", &&dec, &&initDOO, &&initDOO},		//077		4D	DEC		eBP
        {"DEC", &&dec, &&initDOO, &&initDOO},		//078		4E	DEC		eSI
        {"DEC", &&dec, &&initDOO, &&initDOO},		//079		4F	DEC		eDI
        {"PUSH",&&push, &&initDOO, &&initDOO},		//080		50	PUSH	eAX
        {"PUSH", &&push, &&initDOO, &&initDOO},		//081		51	PUSH	eCX
        {"PUSH", &&push, &&initDOO, &&initDOO},		//082		52	PUSH	eDX
        {"PUSH", &&push, &&initDOO, &&initDOO},		//083		53	PUSH	eBX
        {"PUSH", &&push, &&initDOO, &&initDOO},		//084		54	PUSH	eSP
        {"PUSH", &&push, &&initDOO, &&initDOO},		//085		55	PUSH	eBP
        {"PUSH", &&push, &&initDOO, &&initDOO},		//086		56	PUSH	eSI
        {"PUSH", &&push, &&initDOO, &&initDOO},		//087		57	PUSH	eDI
        {"POP", &&pop, &&initDOO, &&initDOO},		//088		58	POP		eAX
        {"POP", &&pop, &&initDOO, &&initDOO},		//089		59	POP		eCX
        {"POP", &&pop, &&initDOO, &&initDOO},		//090		5A	POP		eDX
        {"POP", &&pop, &&initDOO, &&initDOO},		//091		5B	POP		eBX
        {"POP", &&pop, &&initDOO, &&initDOO},		//092		5C	POP		eSP
        {"POP", &&pop, &&initDOO, &&initDOO},		//093		5D	POP		eBP
        {"POP", &&pop, &&initDOO, &&initDOO},		//094		5E	POP		eSI
        {"POP", &&pop, &&initDOO, &&initDOO},		//095		5F	POP		eDI
        {"--", &&label, &&label, &&initlabel},		//096		60	--
        {"--", &&label, &&label, &&initlabel},		//097		61	--
        {"--", &&label, &&label, &&initlabel},		//098		62	--
        {"--", &&label, &&label, &&initlabel},		//099		63	--
        {"--", &&label, &&label, &&initlabel},		//100		64	--
        {"--", &&label, &&label, &&initlabel},		//101		65	--
        {"--", &&label, &&label, &&initlabel},		//102		66	-- Operand-size override prefix
        {"--", &&label, &&label, &&initlabel},		//103		67	-- Address-size override prefix
        {"--", &&label, &&label, &&initlabel},		//104		68	--
        {"--", &&label, &&label, &&initlabel},		//105		69	--
        {"--", &&label, &&label, &&initlabel},		//106		6A	--
        {"--", &&label, &&label, &&initlabel},		//107		6B	--
        {"--", &&label, &&label, &&initlabel},		//108		6C	--
        {"--", &&label, &&label, &&initlabel},		//109		6D	--
        {"--", &&label, &&label, &&initlabel},		//110		6E	--
        {"--", &&label, &&label, &&initlabel},		//111		6F	--
        {"JO", &&label, &&label, &&initlabel},		//112		70	JO		Jb
        {"JNO", &&label, &&label, &&initlabel},		//113		71	JNO		Jb
        {"JB", &&label, &&label, &&initlabel},		//114		72	JB		Jb
        {"JNB", &&label, &&label, &&initlabel},		//115		73	JNB		Jb
        {"JZ", &&label, &&label, &&initlabel},		//116		74	JZ		Jb
        {"JNZ", &&label, &&label, &&initlabel},		//117		75	JNZ		Jb
        {"JBE", &&label, &&label, &&initlabel},		//118		76	JBE		Jb
        {"JA", &&label, &&label, &&initlabel},		//119		77	JA		Jb
        {"JS", &&label, &&label, &&initlabel},		//120		78	JS		Jb
        {"JNS", &&label, &&label, &&initlabel},		//121		79	JNS		Jb
        {"JPE", &&label, &&label, &&initlabel},		//122		7A	JPE		Jb
        {"JPO", &&label, &&label, &&initlabel},		//123		7B	JPO		Jb
        {"JL", &&label, &&label, &&initlabel},		//124		7C	JL		Jb
        {"JGE", &&label, &&label, &&initlabel},		//125		7D	JGE		Jb
        {"JLE", &&label, &&label, &&initlabel},		//126		7E	JLE		Jb
        {"JG", &&label, &&label, &&initlabel},		//127		7F	JG		Jb
        {"GRP1", &&label, &&label, &&initGRP1EbIb},		//128		80	GRP1	Eb	Ib
        {"GRP1", &&label, &&label, &&initlabel},		//129		81	GRP1	Ev	Iv
        {"GRP1", &&label, &&label, &&initlabel},		//130		82	GRP1	Eb	Ib
        {"GRP1", &&label, &&label, &&initGRP1EvIb},		//131		83	GRP1	Ev	Ib
        {"TEST", &&label, &&label, &&initlabel},		//132		84	TEST	Gb	Eb
        {"TEST", &&label, &&label, &&initlabel},		//133		85	TEST	Gv	Ev
        {"XCHG", &&label, &&label, &&initlabel},		//134		86	XCHG	Gb	Eb
        {"XCHG", &&label, &&label, &&initlabel},		//135		87	XCHG	Gv	Ev
        {"MOV", &&label, &&label, &&initTO},		//136		88	MOV		Eb	Gb
        {"MOV", &&movGvGv, &&label, &&initTO},		//137		89	MOV		Ev	Gv
        {"MOV", &&label, &&label, &&initTO},		//138		8A	MOV		Gb	Eb
        {"MOV", &&label, &&label, &&initTO},		//139		8B	MOV		Gv	Ev
        {"MOV", &&label, &&label, &&initlabel},		//140		8C	MOV		Ew	Sw
        {"LEA", &&lea, &&lea, &&initTO},	    	//141		8D	LEA		Gv	M
        {"MOV", &&label, &&label, &&initlabel},		//142		8E	MOV		Sw	Ew
        {"POP", &&label, &&label, &&initlabel},		//143		8F	POP		Ev
        {"NOP", &&label, &&label, &&initlabel},		//144		90	NOP
        {"XCHG", &&xchg, &&initDOO, &&initDOO},		//145		91	XCHG	eCX eAX
        {"XCHG", &&xchg, &&initDOO, &&initDOO},		//146		92	XCHG	eDX eAX
        {"XCHG", &&xchg, &&initDOO, &&initDOO},		//147		93	XCHG	eBX eAX
        {"XCHG", &&xchg, &&initDOO, &&initDOO},		//148		94	XCHG	eSP eAX
        {"XCHG", &&xchg, &&initDOO, &&initDOO},		//149		95	XCHG	eBP eAX
        {"XCHG", &&xchg, &&initDOO, &&initDOO},		//150		96	XCHG	eSI eAX
        {"XCHG", &&xchg, &&initDOO, &&initDOO},		//151		97	XCHG	eDI eAX
        {"CBW", &&label, &&label, &&initlabel},		//152		98	CBW
        {"CWD", &&label, &&label, &&initlabel},		//153		99	CWD
        {"CALL", &&label, &&label, &&initlabel},		//154		9A	CALL	Ap
        {"WAIT", &&label, &&label, &&initlabel},		//155		9B	WAIT
        {"PUSHF", &&label, &&label, &&initlabel},		//156		9C	PUSHF
        {"POPF", &&label, &&label, &&initlabel},		//157		9D	POPF
        {"SAHF", &&label, &&label, &&initlabel},		//158		9E	SAHF
        {"LAHF", &&label, &&label, &&initlabel},		//159		9F	LAHF
        {"MOV", &&label, &&label, &&initlabel},		//160		A0	MOV		AL	Ob
        {"MOV", &&label, &&label, &&initlabel},		//161		A1	MOV		eAX	Ov
        {"MOV", &&label, &&label, &&initlabel},		//162		A2	MOV		Ob	AL
        {"MOV", &&label, &&label, &&initlabel},		//163		A3	MOV		Ov	eAX
        {"MOVSB", &&label, &&label, &&initlabel},		//164		A4	MOVSB
        {"MOVSW", &&label, &&label, &&initlabel},		//165		A5	MOVSW
        {"CMPSB", &&label, &&label, &&initlabel},		//166		A6	CMPSB
        {"CMPSW", &&label, &&label, &&initlabel},		//167		A7	CMPSW
        {"TEST", &&label, &&label, &&initlabel},		//168		A8	TEST	AL	Ib
        {"TEST", &&label, &&label, &&initlabel},		//169		A9	TEST	eAX	Iv
        {"STOSB", &&label, &&label, &&initlabel},		//170		AA	STOSB
        {"STOSW", &&label, &&label, &&initlabel},		//171		AB	STOSW
        {"LODSB", &&label, &&label, &&initlabel},		//172		AC	LODSB
        {"LODSW", &&label, &&label, &&initlabel},		//173		AD	LODSW
        {"SCASB", &&label, &&label, &&initlabel},		//174		AE	SCASB
        {"SCASW", &&label, &&label, &&initlabel},		//175		AF	SCASW
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//176		B0	MOV		AL	Ib
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//177		B1	MOV		CL	Ib
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//178		B2	MOV		DL	Ib
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//179		B3	MOV		BL	Ib
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//180		B4	MOV		AH	Ib
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//181		B5	MOV		CH	Ib
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//182		B6	MOV		DH	Ib
        {"MOV", &&movIb, &&label, &&initImmedMOV},		//183		B7	MOV		BH	Ib
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//184		B8	MOV		eAX	Iv
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//185		B9	MOV		eCX	Iv
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//186		BA	MOV		eDX	Iv
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//187		BB	MOV		eBX	Iv
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//188		BC	MOV		eSP	Iv
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//189		BD	MOV		eBP	Iv
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//190		BE	MOV		eSI	Iv
        {"MOV", &&movIv, &&label, &&initImmedMOV},		//191		BF	MOV		eDI	Iv
        {"--", &&label, &&label, &&initlabel},		//192		C0	--
        {"GRP2", &&sarEv, &&label, &&initGRP2Ev},		//193		C1	--Shift Grp 2 Ev, Ib
        {"RET", &&label, &&label, &&initlabel},		//194		C2	RET		Iw
        {"RET", &&label, &&label, &&initlabel},		//195		C3	RET
        {"LES", &&label, &&label, &&initlabel},		//196		C4	LES		Gv	Mp
        {"LDS", &&label, &&label, &&initlabel},		//197		C5	LDS		Gv	Mp
        {"MOV", &&label, &&label, &&initlabel},		//198		C6	MOV		Eb	Ib
        {"MOV", &&label, &&label, &&initlabel},		//199		C7	MOV		Ev	Iv
        {"--", &&label, &&label, &&initlabel},		//200		C8	--
        {"--", &&label, &&label, &&initlabel},		//201		C9	--
        {"RETF", &&label, &&label, &&initlabel},		//202		CA	RETF	Iw
        {"RETF", &&label, &&label, &&initlabel},		//203		CB	RETF
        {"INT", &&label, &&label, &&initlabel},		//204		CC	INT		3
        {"INT", &&label, &&label, &&initlabel},		//205		CD	INT		Ib
        {"INTO", &&label, &&label, &&initlabel},		//206		CE	INTO
        {"IRET", &&label, &&label, &&initlabel},		//207		CF	IRET
        {"GRP2", &&label, &&label, &&initlabel},		//208		D0	GRP2	Eb	1
        {"GRP2", &&label, &&label, &&initlabel},		//209		D1	GRP2	Ev	1
        {"GRP2", &&label, &&label, &&initlabel},		//210		D2	GRP2	Eb	CL
        {"GRP2", &&label, &&label, &&initlabel},		//211		D3	GRP2	Ev	CL
        {"AAM", &&label, &&label, &&initlabel},		//212		D4	AAM		I0
        {"AAD", &&label, &&label, &&initlabel},		//213		D5	AAD		I0
        {"--", &&label, &&label, &&initlabel},		//214		D6	--
        {"XLAT", &&label, &&label, &&initlabel},		//215		D7	XLAT
        {"--", &&label, &&label, &&initlabel},		//216		D8	--
        {"--", &&label, &&label, &&initlabel},		//217		D9	--
        {"--", &&label, &&label, &&initlabel},		//218		DA	--
        {"--", &&label, &&label, &&initlabel},		//219		DB	--
        {"--", &&label, &&label, &&initlabel},		//220		DC	--
        {"--", &&label, &&label, &&initlabel},		//221		DD	--
        {"--", &&label, &&label, &&initlabel},		//222		DE	--
        {"--", &&label, &&label, &&initlabel},		//223		DF	--
        {"LOOPNZ", &&label, &&label, &&initlabel},		//224		E0	LOOPNZ	Jb
        {"LOOPZ", &&label, &&label, &&initlabel},		//225		E1	LOOPZ	Jb
        {"LOOP", &&label, &&label, &&initlabel},		//226		E2	LOOP	Jb
        {"JCXZ", &&label, &&label, &&initlabel},		//227		E3	JCXZ	Jb
        {"IN", &&label, &&label, &&initlabel},		//228		E4	IN		AL	Ib
        {"IN", &&label, &&label, &&initlabel},		//229		E5	IN		eAX	Ib
        {"OUT", &&label, &&label, &&initlabel},		//230		E6	OUT		Ib	AL
        {"OUT", &&label, &&label, &&initlabel},		//231		E7	OUT		Ib	eAX
        {"CALL", &&label, &&label, &&initlabel},		//232		E8	CALL	Jv
        {"JMP", &&label, &&label, &&initlabel},		//233		E9	JMP		Jv
        {"JMP", &&label, &&label, &&initlabel},		//234		EA	JMP		Ap
        {"JMP", &&label, &&label, &&initlabel},		//235		EB	JMP		Jb
        {"IN", &&label, &&label, &&initlabel},		//236		EC	IN		AL	DX
        {"IN", &&label, &&label, &&initlabel},		//237		ED	IN		eAX	DX
        {"OUT", &&label, &&label, &&initlabel},		//238		EE	OUT		DX	AL
        {"OUT", &&label, &&label, &&initlabel},		//239		EF	OUT		DX	eAX
        {"LOCK", &&label, &&label, &&initlabel},		//240		F0	LOCK
        {"--", &&label, &&label, &&initlabel},		//241		F1	--
        {"REPNZ", &&label, &&label, &&initlabel},		//242		F2	REPNZ
        {"REPZ", &&label, &&label, &&initlabel},		//243		F3	REPZ
        {"HLT", &&label, &&label, &&initlabel},		//244		F4	HLT
        {"CMC", &&label, &&label, &&initlabel},		//245		F5	CMC
        {"GRP3a", &&label, &&label, &&initlabel},		//246		F6	GRP3a	Eb
        {"GRP3b", &&label, &&label, &&initlabel},		//247		F7	GRP3b	Ev
        {"CLC", &&label, &&label, &&initlabel},		//248		F8	CLC
        {"STC", &&label, &&label, &&initlabel},		//249		F9	STC
        {"CLI", &&label, &&label, &&initlabel},		//250		FA	CLI
        {"STI", &&label, &&label, &&initlabel},		//251		FB	STI
        {"CLD", &&label, &&label, &&initlabel},		//252		FC	CLD
        {"STD", &&label, &&label, &&initlabel},		//253		FD	STD
        {"GRP4", &&label, &&label, &&initlabel},		//254		FE	GRP4	Eb
        {"GRP5", &&label, &&label, &&initlabel},		//255		FF	GRP5	Ev
    };

    //dispatch table for grp1
    GRP grp1[] =
    {
        {&&label, &&label, &&label, &&label, &&label, &&label, &&label, &&cmpb},
        {&&label, &&label, &&label, &&label, &&label, &&label, &&label, &&label},
        {&&label, &&label, &&label, &&label, &&label, &&label, &&label, &&label},
        {&&label, &&label, &&label, &&label, &&andGvIb, &&subGvIb, &&label, &&label}
    };

    if ( argc != 2 )
    {
        printf("usage: %s filename", argv[0]);
    }
    else
    {
        FILE *file = fopen(argv[1], "r");
        if (file == 0)
            printf( "Could not open %s\n", argv[1]);
        else
        {
            //Pre-decoding
            char x[2];
            char y = 's';
            int li = 0, ip = 0, mod, reg, rm, d, w, i, opext;
            OP op;
            while  (y != EOF && y != '\n')
            {
                li = strtol(fgets(x,3,file), NULL, 16);
                printf("Reading instruction...\n");
                printf("Character read: %s -> %d (%d)\n", x, li, ip);
                code[ip].op = li;
                op = dispatch[li];
                code[ip].label = op.op1label;
                goto *op.initlabel;
                next:
                ip++;
                y = fgetc(file);
            }
            fclose( file );

            goto start; //Launch the predecoded program

            //Initialization labels
            initImmediate: //Initialisation for immediate instructions
                printf("Initialization...\n%s\nImmediate operation\n", op.name);
                w = li%2;
                i = 1 + 3*w;
                code[ip].length = 2 + i;
                code[ip].disp = getdisp(file, i);
                goto next;

            initDOO: //Initialisation for one operand operations
                reg = li%8;
                printf("Initialization...\n%s %s\nDirect one-operand operation\nRegister: %s (%d)\n", op.name, regnames[reg], regnames[reg], reg);
                code[ip].dest = &registers[reg]; //It's not always the destination, but it is useless to define several initialization for push and pop for example
                //(*code[ip].dest)++;
                //printf("%s : %d\n", regnames[reg], registers[reg]);

                goto next;

            initGRP1EvIb: //I did that one at the end, it only takes into account the case in the demo program
                fgetc(file);
                li = strtol (fgets(x,3,file),NULL,16);
                reg = li%8;
                li /= 8;
                opext = li%8;
                code[ip].label = grp1[3].label[opext];
                mod = li / 8;
                printb(opext, "opext");
                printf("Initialization...\nGRP1\nEv Immediate operation\n", op.name);
                if(mod == 3){ // I only took care of the instructions used in the program (SUB, AND)
                    code[ip].dest = &registers[reg];
                    fgetc(file);
                    code[ip].disp = strtol(fgets(x,3,file),NULL,16);
                }
                goto next;

            initGRP1EbIb: // TODO
                fgetc(file);
                li = strtol (fgets(x,3,file),NULL,16);
                i = li;
                reg = li%8;
                li /= 8;
                opext = li%8;
                code[ip].label = grp1[0].label[opext];
                mod = li / 8;
                printf("Initialization...\nGRP1\nEb Immediate operation\n", op.name);

                if(i == 124){ // This is 7C: COMPB only
                    code[ip].length = 5;
                    printf("SIB\n");
                    fgetc(file);
                    li = strtol (fgets(x,3,file),NULL,16);
                    int base = li%8;
                    li /=8;
                    int index = li%8;
                    int scale = pow2(li/8);
                    code[ip].scale = scale;
                    if(base != 5)
                        code[ip].src1 = &registers[base];
                    else
                        code[ip].src1 = &zero;

                    if(index != 4)
                        code[ip].src2 = &registers[index];
                    else
                        code[ip].src2 = &zero;
                    code[ip].disp = getdisp(file, 1);
                    code[ip].imd = getdisp(file, 1);
                }
                goto next;

            initGRP2Ev: // TODO
                printf("Initialization...\nGRP2\nImmediate operation\n", op.name);
                fgetc(file);
                li = strtol (fgets(x,3,file),NULL,16);
                if(li >= 8){ // SAR Gv Ib only
                    fgetc(file);
                    code[ip].dest = &registers[li%8];
                    code[ip].disp = strtol(fgets(x,3,file),NULL,16);
                }
                goto next;

            initTO: //Two operand operations
                //fgets(x,3,file);
                w = li%2;
                d = (li/2)%2;
                fgetc(file);
                li = strtol (fgets(x,3,file),NULL,16);
                rm = li%8;
                li /= 8;
                reg = li%8;
                mod = li / 8;
                printf("Initialization...\n%s\nDirect two-operand operation\nw: %d\nd: %d\nreg: %d\nrm: %d\nmod: %d\n", op.name, w, d, reg, rm, mod);
                code[ip].length = 2;
                if(mod == 3){
                    //Two general purpose register
                    unsigned int *regpt, *rmpt;
                    int regstr = 0, rmstr = 0;
                    if(w == 0){
                        regpt = &registers[reg%4];
                        rmpt = &registers[rm%4];
                        regstr = (reg > 3) * 8;
                        rmstr = (rm > 3) * 8;
                    }
                    else{
                        regpt = &registers[reg];
                        rmpt = &registers[rm];
                    }
                    if(d){
                            code[ip].dest = regpt;
                            code[ip].src1 = rmpt;
                            code[ip].deststr = regstr;
                            code[ip].src1str = rmstr;
                    }
                    else{
                            code[ip].dest = rmpt;
                            code[ip].src1 = regpt;
                            code[ip].deststr = rmstr;
                            code[ip].src1str = regstr;
                    }
                }
                else{
                    code[ip].label = op.op2label;

                    int regstr = 0, rmstr = 0;
                    if(w == 0){
                        code[ip].reg = &registers[reg%4];
                        code[ip].regstr = (reg > 3) * 8;
                    }
                    else
                        code[ip].reg = &registers[reg];
                    if(w == 0){
                        code[ip].twoSrc = !(rm/4);
                        if(code[ip].twoSrc){
                            if(rm/2 == 0)
                                code[ip].src1 = &BX;
                            else
                                code[ip].src1 = &BP;

                            if(rm%2 == 0)
                                code[ip].src2 = &SI;
                            else
                                code[ip].src2 = &DI;
                        }
                        /*else{
                            /*if(rm == 7)
                                code[ip].src1 = &BX;
                            else if(rm == 6){
                                if(mod != 0)
                                    code[ip].src1 = &BP;
                                else
                                    ; //TODO mod = 0 with (BP) + displacement
                            }
                            else
                                code[ip].src1 = &registers[11 - rm];
                        }*/
                    }
                    else{
                        if(rm == 4) {//r/m = 100b, SIB should be there;
                            code[ip].length++;
                            printf("SIB\n");
                            fgetc(file);
                            li = strtol (fgets(x,3,file),NULL,16);
                            int base = li%8;
                            li /=8;
                            int index = li%8;
                            int scale = pow2(li/8);
                            code[ip].scale = scale;
                            if(base != 5)
                                code[ip].src1 = &registers[base];
                            else
                                code[ip].src1 = &zero;

                            if(index != 4)
                                code[ip].src2 = &registers[index];
                            else
                                code[ip].src2 = &zero;
                        }
                        else{
                            code[ip].src1 = &registers[rm]; //base
                            code[ip].src2 = &zero; //index
                            code[ip].scale = 0; //scale
                        }
                        printb(*code[ip].src2, "src2");
                    }
                    i = mod;
                    if(mod == 2)
                        i = 4;
                    code[ip].length += i;
                    code[ip].disp = getdisp(file, i);
                    printb(code[ip].disp, "displacement");
                }
                goto next;

            initImmedMOV: //Immediate mov initialisation
                printf("Initialization... MOV\n");

                reg = li%8;
                w = li%16 > 7;
                i = 1 + 3*w;
                code[ip].length = 2 + i;
                code[ip].disp = getdisp(file, i);

                if(w) //word version
                    code[ip].dest = &registers[reg];
                else{ //8-bit version
                    code[ip].dest = &registers[reg%4];
                    code[ip].deststr = (reg > 3) * 8;
                }

                goto next;


            initlabel: //Unknown operation
                printf("Initialization... Error, unknown opcode\n");
                goto next;

            start:
                code[ip].label = &&end; //We add the end of the program
        }

        printf("Starting program...\n");
        int TPC = 0, SPC = 0;

        //Some values which are used to calculate
        unsigned int *src1, *src2, *dest, deststr;
        unsigned char b1, b2;
        unsigned short s1, s2;
        unsigned int i1, i2;
        INSTR instr;

        instr = code[TPC];
        printregs();
        goto *instr.label;

        //Instruction labels
        inc:
            printf("Execution INC...\n");
            (*instr.dest)++;

            //Fetching the next instruction
            *EIP += 1;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC]; // TODO : use pointer instead
            //The table lookup would be necessary for Jumps
            printregs();
            goto *instr.label;

        dec:
            printf("Execution DEC...\n");
            (*instr.dest)--;

            *EIP += 1;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        pop:
            printf("Execution POP...\n");
            *instr.dest = get32mem(*ESP);
            *ESP += 4;

            *EIP += 1;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        push:
            printf("Execution PUSH...\n");
            i1 = *ESP - 4;
            //printb(get32mem(i1), "before push");
            set32mem(*instr.dest, i1);
            //printb(get32mem(i1), "after push");
            *ESP = i1;

            *EIP += 1;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        xchg:
            printf("Execution XCHG...\n");
            i1 = *instr.dest;
            *instr.dest = *EAX;
            *EAX = i1;

            *EIP += 1;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addGbGb: //Add two values from one register to another (byte)
            printf("Execution ADD Gb Gb...\n");
            dest = instr.dest;
            src1 = instr.src1;
            //*dest += *src1;
            deststr = instr.deststr;
            b1 = (*dest << (24 - deststr)) >> 24; //we are interested in a 8 bits not all of them
            b2 = (*src1 << (24 - instr.src1str)) >> 24; //Using b1, b2, we make sure that if the result is bigger than FF
            //then it's not taken into account

            /*printb(b1,"b1");
            printb(b2,"b2");
            printb(*dest,"*dest");*/

            *dest -= b1 << deststr; //we want to prevent the fact that the result can be more than FF and influence the rest of the real register

            /*printb(b1 << deststr,"b1 << deststr");
            printb(*dest,"*dest");*/

            b1 += b2; //We make sure that this operation works on bytes and not on integers
            *dest += b1 << deststr;

            /*printb(b1,"b1 + b2");
            printb(b1 << deststr,"(b1 + b2) << deststr");
            printb(*dest,"*dest");*/

            *EIP += 2;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addGvGv: //Add two values from one register to another (32-bit word)
            printf("Execution ADD Gv Gv (32)...\n");
            /*printb(*instr.dest,"*dest");
            printb(*instr.src1,"*src1");*/
            *instr.dest += *instr.src1; //It works on the real register so it's simple this time
            //printb(*instr.dest,"*dest");
            *EIP += 2;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addGvGv16: //Add two values from one register to another (16-bit word): not used, no prefix is implemented in the predecoding
            printf("Execution ADD Gv Gv (16)...\n");
            dest = instr.dest;
            src1 = instr.src1;
            s1 = *dest; //It gets rid of the 16 high values bit that way (s stands for short, so 16 bits)
            s2 = *src1;
            /*printb(*dest,"*dest");
            printb(s1,"s1");
            printb(*src1,"*src1");
            printb(s2,"s2");
            printb(*dest,"*dest");*/
            *dest -= s1;
            //printb(*dest,"*dest");
            s1 += s2;
            *dest += s1;
            /*printb(s1,"s1 + s2");
            printb(*dest,"*dest");*/

            *EIP += 2;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addGvAv: //Add two values from one register to memory (32-bit word)
            printf("Execution ADD Gv Av (32)...\n");
            i1 = *instr.src1 + *instr.src2 * instr.scale + instr.disp; //SIB
            //printb(i1,"i1");
            i2 = get32mem(i1);
            //printb(get32mem(i1),"memory[i1]");
            //printb(*instr.reg,"*instr.reg");
            i2 += *instr.reg;
            set32mem(i2, i1);
            //printb(get32mem(i1),"memory[i1]");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addAvGv: //Add two values from memory to register (32-bit word)
            printf("Execution ADD Av Gv (32)...\n");
            i1 = *instr.src1 + *instr.src2 * instr.scale + instr.disp;
            //printb(i1,"i1");
            //printb(get32mem(i1),"memory[i1]");
            //printb(*instr.reg,"*instr.reg");
            *instr.reg += get32mem(i1);
            //printb(*instr.reg,"*instr.reg");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addAbGb: //Add two values from memory to register (byte)
            printf("Execution ADD Ab Gb...\n");
            i1 = *instr.src1;
            //printb(i1,"i1");
            //printb(instr.disp,"instr.disp");
            i1 += instr.disp;
            //printb(i1,"i1");
            b1 = memory[(i1-MEMORY_START)%MEMORY_SIZE];
            //printb(memory[i1%MEMORY_SIZE],"memory[i1%MEMORY_SIZE]");
            //printb(b1,"b1");
            b2 = (*instr.reg << (24 - instr.regstr)) >> 24;
            //printb(*instr.reg,"*instr.reg");
            //printb(b2,"b2");
            *instr.reg -= b2 << instr.regstr;
            //printb(*instr.reg,"*instr.reg");
            b1 += b2;
            //printb(b1,"b1");
            *instr.reg += b1 << instr.regstr;
            //printb(*instr.reg,"*instr.reg");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addGbAb: //Add two values from register to memory (byte)
            printf("Execution ADD Gb Ab...\n");
            i1 = *instr.src1;
            //printb(i1,"i1");
            //printb(instr.disp,"instr.disp");
            i1 += instr.disp;
            //printb(i1,"i1");
            unsigned int *mem = &memory[(i1-MEMORY_START)%MEMORY_SIZE];
            b1 = *mem;
            //printb(*mem,"memory[i1%MEMORY_SIZE]");
            memory[(i1-MEMORY_START)%MEMORY_SIZE] -= b1;
            //printb(*mem,"memory[i1%MEMORY_SIZE]");
            //printb(b1,"b1");
            b2 = (*instr.reg << (24 - instr.regstr)) >> 24;
            //printb(*instr.reg,"*instr.reg");
            //printb(b2,"b2");
            b1 += b2;
            //printb(b1,"b1");
            *mem += b1;
            //printb(*mem,"memory[i1%MEMORY_SIZE]");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addIbAL: //Add immediate value to AL
            printf("Execution ADD Ib AL...\n");
            b1 = *AL;
            //printb(*AL,"EAX");
            //printb(b1,"b1");
            *EAX -= b1;
            //printb(*AL,"EAX");
            //printb(instr.disp,"instr.disp");
            b1 += instr.disp;
            //printb(b1,"b1");
            *EAX += b1;
            //printb(*EAX,"EAX");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        addIvEAX: //Add immediate value to EAX
            printf("Execution ADD Iv eAX...\n");
            //printb(*EAX,"EAX");
            //printb(instr.disp,"instr.disp");
            *EAX += instr.disp;
            //printb(*EAX,"EAX");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        movIv: //Move immediate value to register
            printf("Execution MOV Iv...\n");
            //printb(*instr.dest,"*instr.dest");
            //printb(instr.disp,"instr.dest");
            *instr.dest = instr.disp;
            //printb(*instr.dest,"*instr.dest");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        movIb: //Move immediate value to register (byte)
            printf("Execution MOV Ib...\n");
            //printb(b1,"b1");
            b1 = (*instr.dest << (24 - instr.deststr)) >> 24;
            //printb(*instr.dest,"*instr.dest");
            //printb(b1,"b1");
            *instr.dest -= b1 << instr.deststr;
            //printb(*instr.dest,"*instr.dest");
            b1 = instr.disp;
            //printb(instr.disp,"instr.disp");
            //printb(b1,"b1");
            *instr.dest += b1 << instr.deststr;
            //printb(*instr.dest,"*instr.dest");


            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        sarEv: //Shift right (immediate byte) from register (word)
            printf("Execution SAR Ev Ib...\n");
            //printb(*instr.dest,"*instr.dest");
            //printb(instr.disp,"instr.disp");
            *instr.dest >>= instr.disp;
            //printb(*instr.dest,"*instr.dest");

            *EIP += 3;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        lea:
        printf("Execution LEA...\n");
            /*printb(*instr.reg, "*instr.reg");
            printb(*(&zero), "*(&zero)");
            printb(*instr.src1, "*instr.src1");
            printb(*instr.src2, "*instr.src2");
            printb(instr.scale, "*instr.scale");
            printb(instr.disp, "*instr.disp");
            printb(*instr.src2 * instr.scale, "*instr.src2 * instr.scale");
            printb(*instr.src1 + *instr.src2 * instr.scale, "*instr.src1 + *instr.src2 * instr.scale");*/

            *instr.reg = *instr.src1 + *instr.src2 * instr.scale + instr.disp;
            //printb(*instr.reg, "*instr.reg");

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        cmpb:
            printf("Execution CMPB...\n");
            b1 = (*EFLAGS << 26) >> 32; //Zero flag
            *EFLAGS -= b1 << 6; //Zero flag
            i1 = *instr.src1 + *instr.src2 * instr.scale + instr.disp;
            //printb(i1,"i1");
            i2 = get32mem(i1);
            //printb(get32mem(i1),"memory[i1]");
            *EFLAGS += i2 == instr.imd;
            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        andGvIb:
            printf("Execution AND Gv Ib...\n");
            *instr.dest &= instr.disp; //It's working on a word, unlike SUB Gb Ib
            *EIP += 3;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        subGvIb:
            printf("Execution SUB Gv Ib...\n");
            *instr.dest -= instr.disp; //It's working on a word, unlike SUB Gb Ib
            *EIP += 3;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        movGvGv:
            printf("Execution MOV Gv Gv (32)...\n");
            *instr.dest = *instr.src1;
            *EIP += 2;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

        xorIbAL:
            printf("Execution XOR Ib AL...\n");
            b1 = *AL;
            *EAX -= b1;
            b1 ^= instr.disp;
            *EAX += b1;

            *EIP += instr.length;
            TPC++;
            if (halt || interrupt) goto end;
            instr = code[TPC];
            printregs();
            goto *instr.label;

    }


    label:
    printf("Error...\n");
    end:
    printf("Done.");
}
