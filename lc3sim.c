#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3 machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* 
  MEMORY[A] stores the word address A
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3 State info.                                           */
/***************************************************************/
#define LC_3_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3 ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3 for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3 until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = start ; address <= stop ; address++)
    printf("  0x%.4x (%d) : 0x%.2x\n", address , address , MEMORY[address]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = start ; address <= stop ; address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x\n", address , address , MEMORY[address]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word ;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii] = word;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = program_base;

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3 Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     

  //fetch
  int instr = MEMORY[CURRENT_LATCHES.PC];
  int opcode = (instr >> 12) & 0xF;

  //decode and execute
  switch (opcode) {
    //add
    case 0x1: {
      int dr = (instr >> 9) & 0x7;
      int sr1 = (instr >> 6) & 0x7;
      int imm_flag = (instr >> 5) & 0x1;
      int result;
      //imm
      if (imm_flag) {
        int imm5 = instr & 0x1F;
        //sign
        imm5 = (imm5 & 0x10) ? (imm5 | 0xFFE0) : imm5;
        result = Low16bits(CURRENT_LATCHES.REGS[sr1] + imm5);
      } 
      //sr
      else {
        int sr2 = instr & 0x7;
        result = Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]);
      }
      NEXT_LATCHES.REGS[dr] = result;

      //nzp
      NEXT_LATCHES.N = (result >> 15) & 0x1;
      NEXT_LATCHES.Z = (result == 0);
      NEXT_LATCHES.P = !(NEXT_LATCHES.N || NEXT_LATCHES.Z);
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }
    
    //and

    case 0x5: {
      int dr = (instr >> 9) & 0x7;
      int sr1 = (instr >> 6) & 0x7;
      int imm_flag = (instr >> 5) & 0x1;
      int result;

      if (imm_flag) {
        int imm5 = instr & 0x1F;
        imm5 = (imm5 & 0x10) ? (imm5 | 0xFFE0) : imm5;
        result = Low16bits(CURRENT_LATCHES.REGS[sr1] & imm5);
      } else {
        int sr2 = instr & 0x7;
        result = Low16bits(CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2]);
      }
      NEXT_LATCHES.REGS[dr] = result;

      // nzp
      NEXT_LATCHES.N = (result >> 15) & 0x1;
      NEXT_LATCHES.Z = (result == 0);
      NEXT_LATCHES.P = !(NEXT_LATCHES.N || NEXT_LATCHES.Z);
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    //br

    case 0x0: { 
      int nzp = (instr >> 9) & 0x7;
      int pc_offset = instr & 0x1FF;
      pc_offset = (pc_offset & 0x100) ? (pc_offset | 0xFE00) : pc_offset;

      if ((nzp & 0x4 && CURRENT_LATCHES.N) || 
          (nzp & 0x2 && CURRENT_LATCHES.Z) || 
          (nzp & 0x1 && CURRENT_LATCHES.P)) {
        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + pc_offset + 1);
      } else {
        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      }
      break;
    }

    //jmp
    case 0xC: {
      int base_reg = (instr >> 6) & 0x7;
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[base_reg]);
      break;
    }
    //jsr
        case 0x4: { 
      NEXT_LATCHES.REGS[7] = Low16bits(CURRENT_LATCHES.PC+1);
      if ((instr >> 11) & 0x1) { //jsr
        int offset = instr & 0x7FF;
        offset = (offset & 0x400) ? (offset | 0xF800) : offset; 
        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + offset + 1);
      } else { //jsrr
        int base = (instr >> 6) & 0x7;
        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[base]);
      }
      break;
    }

    case 0x2: { // ld
      int dr = (instr >> 9) & 0x7;
      int offset = instr & 0x1FF;
      offset = (offset & 0x100) ? (offset | 0xFE00) : offset;
      NEXT_LATCHES.REGS[dr] = MEMORY[Low16bits(CURRENT_LATCHES.PC + offset + 1)];
      // nzp
      NEXT_LATCHES.N = (NEXT_LATCHES.REGS[dr] >> 15) & 0x1;
      NEXT_LATCHES.Z = (NEXT_LATCHES.REGS[dr] == 0);
      NEXT_LATCHES.P = !(NEXT_LATCHES.N || NEXT_LATCHES.Z);
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    case 0xA: { // ldi
      int dr = (instr >> 9) & 0x7;
      int offset = instr & 0x1FF;
      offset = (offset & 0x100) ? (offset | 0xFE00) : offset;
      int addr = MEMORY[Low16bits(CURRENT_LATCHES.PC + offset + 1)];
      NEXT_LATCHES.REGS[dr] = MEMORY[addr];
      // nzp
      NEXT_LATCHES.N = (NEXT_LATCHES.REGS[dr] >> 15) & 0x1;
      NEXT_LATCHES.Z = (NEXT_LATCHES.REGS[dr] == 0);
      NEXT_LATCHES.P = !(NEXT_LATCHES.N || NEXT_LATCHES.Z);
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    case 0x6: { // ldr
      int dr = (instr >> 9) & 0x7;
      int base = (instr >> 6) & 0x7;
      int offset = instr & 0x3F;
      offset = (offset & 0x20) ? (offset | 0xFFC0) : offset;
      NEXT_LATCHES.REGS[dr] = MEMORY[Low16bits(CURRENT_LATCHES.REGS[base] + offset)];
      // nzp
      NEXT_LATCHES.N = (NEXT_LATCHES.REGS[dr] >> 15) & 0x1;
      NEXT_LATCHES.Z = (NEXT_LATCHES.REGS[dr] == 0);
      NEXT_LATCHES.P = !(NEXT_LATCHES.N || NEXT_LATCHES.Z);
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    case 0xE: { // lea
      int dr = (instr >> 9) & 0x7;
      int offset = instr & 0x1FF;
      offset = (offset & 0x100) ? (offset | 0xFE00) : offset;
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.PC + offset + 1);
 
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    case 0x9: { // not
      int dr = (instr >> 9) & 0x7;
      int sr = (instr >> 6) & 0x7;
      NEXT_LATCHES.REGS[dr] = Low16bits(~CURRENT_LATCHES.REGS[sr]);
      // nzp
      NEXT_LATCHES.N = (NEXT_LATCHES.REGS[dr] >> 15) & 0x1;
      NEXT_LATCHES.Z = (NEXT_LATCHES.REGS[dr] == 0);
      NEXT_LATCHES.P = !(NEXT_LATCHES.N || NEXT_LATCHES.Z);
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }


    case 0x3: { // st
      int sr = (instr >> 9) & 0x7;
      int offset = instr & 0x1FF;
      offset = (offset & 0x100) ? (offset | 0xFE00) : offset;
      MEMORY[Low16bits(CURRENT_LATCHES.PC + offset + 1)] = CURRENT_LATCHES.REGS[sr];
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    case 0xB: { // sti
      int sr = (instr >> 9) & 0x7;
      int offset = instr & 0x1FF;
      offset = (offset & 0x100) ? (offset | 0xFE00) : offset;
      int addr = MEMORY[Low16bits(CURRENT_LATCHES.PC + offset + 1)];
      MEMORY[addr] = CURRENT_LATCHES.REGS[sr];
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    case 0x7: { // str
      int sr = (instr >> 9) & 0x7;
      int base = (instr >> 6) & 0x7;
      int offset = instr & 0x3F;
      offset = (offset & 0x20) ? (offset | 0xFFC0) : offset;
      MEMORY[Low16bits(CURRENT_LATCHES.REGS[base] + offset)] = CURRENT_LATCHES.REGS[sr];
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
    }

    //trap
    case 0xF: {
      NEXT_LATCHES.REGS[7] = Low16bits(CURRENT_LATCHES.PC + 1);
      NEXT_LATCHES.PC = Low16bits(MEMORY[instr& 0xFF]);
      break;
    }

    default:
      printf("Unknown opcode: 0x%X\n", opcode);
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 1);
      break;
  }

}
