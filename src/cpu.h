 #pragma once
 
#include "typedef.h"
#include "bus.h" 


typedef struct 
{
   u32 reg;
   u32 cur;
   u32 prev;
}delay;


/////////////////// Coprocessor 0

typedef union
{
    struct 
    {
      u32 interrupt_enable : 1;    //   0     IEc Current Interrupt Enable  (0=Disable, 1=Enable) ;rfe pops IUp here
      u32 current_kernel_mode : 1; //   1     KUc Current Kernel/User Mode  (0=Kernel, 1=User)    ;rfe pops KUp here
      u32 prev_interrupt : 1;      //   2     IEp Previous Interrupt Disable                      ;rfe pops IUo here
      u32 prev_kernel : 1;         //   3     KUp Previous Kernel/User Mode                       ;rfe pops KUo here
      u32 interrupt_disable : 1;   //   4     IEo Old Interrupt Disable                       ;left unchanged by rfe
      u32 old_kernel : 1;          //   5     KUo Old Kernel/User Mode                        ;left unchanged by rfe
      u32 not_used_6_7 : 2;        //   6-7   -   Not used (zero)
      u32 interrupt_mask : 8;      //   8-15  Im  8 bit interrupt mask fields. When set the corresponding
      u32 isolate_cache : 1;       //   16    Isc Isolate Cache (0=No, 1=Isolate)
      u32 swapped_cache : 1;       //   17    Swc Swapped cache mode (0=Normal, 1=Swapped)
      u32 pz : 1;  //   18    PZ  When set cache parity bits are written as 0.
      u32 cm : 1;  //   19    CM  Shows the result of the last load operation with the D-cache
      u32 cache_parity_err : 1;   //  20    PE  Cache parity error (Does not cause exception)
      u32 tlb_shutdown : 1;       //  21    TS  TLB shutdown. Gets set if a programm address simultaneously
      u32 boot_exception : 1;     //  22    BEV Boot exception vectors in RAM/ROM (0=RAM/KSEG0, 1=ROM/KSEG1)
      u32 not_used_23_24 : 2;     //  23-24 -   Not used (zero)
      u32 reverse_endianness : 1; //  25    RE  Reverse endianness   (0=Normal endianness, 1=Reverse endianness)
      u32 not_used_26_27 : 2;     //  26-27 -   Not used (zero)
      u32 cop0_enable : 4;        // 28    CU 0 1 2 3 COP0 Enable (0=Enable only in Kernel Mode, 1=Kernel and User Mode)
    };
    u32 word;
  
}status;


typedef union
{
     struct 
    {
      u32 not_used_0_2 : 2;          // 0-1   -      Not used (zero)
      u32 excode : 5;                // 2-6   Excode Describes what kind of exception occured: Table 9.53 Cause Register ExcCode Field
      u32 not_used_7 : 1;            // 7     -      Not used (zero)
      u32 interrupt_pending : 8 ;    // 8-15  Ip     Interrupt pending field. Bit 8 and 9 are R/W, and
      u32 not_used_16_27 : 12;       // 16-27 -      Not used (zero)
      u32 coprocessor_exception : 2; // 28-29 CE     Contains the coprocessor number if the exception
      u32 not_used_30 : 1;           // 30    -      Not used (zero)
      u32 branch_delay_slot : 1;     // 31    BD     Is set when last exception points to the

    };
    
    u32 word;
  
}cause;

enum COP0
{
  BPC       = 0x3,      // cop0r3      - BPC - Breakpoint on execute (R/W)
  BDA       = 0x5,      // cop0r5      - BDA - Breakpoint on data access (R/W)
  JUMPDEST  = 0x6,      // cop0r6      - JUMPDEST - Randomly memorized jump address (R)
  DCIC      = 0x7,      // cop0r7      - DCIC - Breakpoint control (R/W)
  BadVaddr  = 0x8,      // cop0r8      - BadVaddr - Bad Virtual Address (R)
  BDAM      = 0x9,      // cop0r9      - BDAM - Data Access breakpoint mask (R/W)
  BPCM      = 0xb,      // cop0r11     - BPCM - Execute breakpoint mask (R/W)
  SR        = 0xc,      // cop0r12     - SR - System status register (R/W)
  CAUSE     = 0xd,      // cop0r13     - CAUSE - (R)  Describes the most recently recognised exception
  EPC       = 0xe,      // cop0r14     - EPC - Return Address from Trap (R)
  PRID      = 0xf,      // cop0r15     - PRID - Processor ID (R)
 
  // cop0r0-r2   - N/A
  // cop0r4      - N/A
  // cop0r10     - N/A
  // cop0r16-r31 - Garbage
  // cop0r32-r63 - N/A - None such (Control regs)
};

enum MODE
{
  KERNEL = 0, // Kernel Mode
  USER = 1,   // User Mode
};

typedef struct CPU
{

/*
  Name       Alias    Common Usage
  R0         zero     Constant (always 0)
  R1         at       Assembler temporary (destroyed by some assembler pseudoinstructions!)
  R2-R3      v0-v1    Subroutine return values, may be changed by subroutines
  R4-R7      a0-a3    Subroutine arguments, may be changed by subroutines
  R8-R15     t0-t7    Temporaries, may be changed by subroutines
  R16-R23    s0-s7    Static variables, must be saved by subs
  R24-R25    t8-t9    Temporaries, may be changed by subroutines
  R26-R27    k0-k1    Reserved for kernel (destroyed by some IRQ handlers!)
  R28        gp       Global pointer (rarely used)
  R29        sp       Stack pointer
  R30        fp(s8)   Frame Pointer, or 9th Static variable, must be saved
  R31        ra       Return address (used so by JAL,BLTZAL,BGEZAL opcodes)
  -          pc       Program counter
  -          hi,lo    Multiply/divide results, may be changed by subroutines
*/

// CPU Registers
// All registers are 32bit wide.
u32 gpr_reg[32]; // General Purpose Register [GPR]

u32 pc;  // program counter

u32 next_pc; // next instr 

u32 pc_exception; // used for exception

u32 hi,lo; // Multiply/divide results, may be changed by subroutines

//  Opcode/Parameter Encoding

u32 opcode;

bool delay_slot;

delay slot_next;

delay slot_cur;

//  COP0 System Control Coprocessor           - 32 registers (not all used)

u32 m_cop0_bpc;      // cop0r3    - BPC - Breakpoint on execute (R/W)
u32 m_cop0_bda;      // cop0r5    - BDA - Breakpoint on data access (R/W)
u32 m_cop0_jmptest;  // cop0r6    - JUMPDEST - Randomly memorized jump address (R)
u32 m_cop0_dcic;     // cop0r7    - DCIC - Breakpoint control (R/W)
u32 m_cop0_badvaddr; // cop0r8    - BadVaddr - Bad Virtual Address (R)
u32 m_cop0_bdam;     // cop0r9    - BDAM - Data Access breakpoint mask (R/W)
u32 m_cop0_bpcm;     // cop0r11   - BPCM - Execute breakpoint mask (R/W)
status m_cop0_sr;    // cop0r12   - SR - System status register (R/W)
cause m_cop0_cause;  // cop0r13   - CAUSE - (Read-only, except, Bit8-9 are R/W)
u32 m_cop0_epc;      // cop0r14   - EPC - Return Address from Trap (R)
u32 m_cop0_prid;     // cop0r15   - PRID - Processor ID (R) 


bool branch_delay_slot_saved;

bool branch_taken_saved;


}R3000;

/* Number Mnemonic Description
0 INT External Interrupt
1 MOD TLB Modification Exception
2 TLBL TLB miss Exception (Load or instruction fetch)
3 TLBS TLB miss exception (Store)
4 ADEL Address Error Exception (Load or instruction fetch)
5 ADES Address Error Exception (Store)
6 IBE Bus Error Exception (for Instruction Fetch)
7 DBE Bus Error Exception (for data Load or Store)
8 SYS SYSCALL Exception
9 BP Breakpoint Exception
10 RI Reserved Instruction Exception
11 CPU Co-Processor Unusable Exception
12 OVF Arithmetic Overflow Exception
13-31 - Reserved */

 enum Exception
{
  Interrupt      = 0x00, // 00h INT     Interrupt
  LoadAddrError  = 0x04, // 04h AdEL    Address error, Data load or Instruction fetch
  WriteAddrError = 0x05, // 05h AdES    Address error, Data store
  LoadBusError   = 0x06, // 06h IBE     Bus error on Instruction fetch
  WriteBusError  = 0x07, // 07h DBE     Bus error on Data load/store
  Syscall        = 0x08, // 08h Syscall Generated unconditionally by syscall instruction
  BreakPoint     = 0x09, // 09h BP      Breakpoint - break instruction
  ReserveInst    = 0x0a, // 0Ah RI      Reserved instruction
  Overflow       = 0x0c, // 0Ch Ov      Arithmetic Overflow Ovf Twos complement overflow during add or subtract
};


char* namereg(u8 index);

void reset_cpu(R3000 *cpu);

void execute_cpu(R3000 *cpu);

void special(R3000 *cpu);

u32 opcode(R3000 *cpu); // aka op prim

u32 function(R3000 *cpu); // aka op sec

// Causa la segnalazione di un'eccezione, utilizzando il parametro exception come tipo di eccezione e il parametro argument come
// argomento specifico dell'eccezione). Il controllo non ritorna da questa funzione di pseudocodice: l'eccezione viene segnalata al
// momento della chiamata
void signalException(R3000 *cpu,u8 cause);

void JumpDelaySlot(R3000* cpu,u32 reg,u32 value);

void load_update_badvaddr(R3000 *cpu,u32 addr);
void write_update_badvaddr(R3000 *cpu,u32 addr);

u32 rs(R3000 *cpu);

u32 rt(R3000 *cpu);

u32 rd(R3000 *cpu);

u32 shift(R3000 *cpu); // aka imm5

u32 imm16(R3000 *cpu);

s32 imm16sign(R3000 *cpu);

u32 imm20(R3000 *cpu);

u32 imm25(R3000 *cpu);
  
u32 target(R3000 *cpu); // aka imm26

u32 gpr(R3000 *cpu, u8 index);

u32 set_gpr(R3000 *cpu, u8 index,u8 v);

u32 set_rs(R3000 *cpu,u8 value);

u32 set_rt(R3000 *cpu,u8 value);

u32 set_rd(R3000 *cpu,u8 value);


void jump_addr(R3000 *cpu,u32 addr);

void set_pc(R3000 *cpu,u32 addr);

// instruction

// Table 3-1 CPU Arithmetic Instructions
void add(R3000 *cpu);    // Add Word
void addi(R3000 *cpu);   // Add Immediate Word
void addiu(R3000 *cpu);  // Add Immediate Unsigned Word
void addu(R3000 *cpu);   // Add Unsigned Word

void div_(R3000 *cpu);   // Divide Word
void divu(R3000 *cpu);   // Divide Unsigned Word
void mult(R3000 *cpu);   // Multiply Word to GPR
void multu(R3000 *cpu);  // Multiply Unsigned Word
void slt(R3000 *cpu);    // Set on Less Than 
void sltu(R3000 *cpu);   // Set on Less Than Unsigned
void slti(R3000 *cpu);   // Set on Less Than Immediate
void sltiu(R3000 *cpu);  // Set on Less Than Immediate Unsigned
void sub(R3000 *cpu);    // Subtract Word
void subu(R3000 *cpu);   // Subtract Unsigned Word


void bcondz(R3000 *cpu); // Unconditional Branch

// Table 3-2 CPU Branch and Jump Instructions
void beq(R3000 *cpu);  // Branch on Equal
void bgtz(R3000 *cpu); // Branch on Greater Than or Equal to Zero
void blez(R3000 *cpu); // Branch on Less Than or Equal to Zero
void bne(R3000 *cpu);  // Branch on Not Equal
void j(R3000 *cpu); // Jump
void jal(R3000 *cpu); // Jump and Link
void jalr(R3000 *cpu); // Jump and Link Register
void jr(R3000 *cpu); // Jump Register

//  Table 3-3 CPU Instruction Control Instructions
void nop(R3000 *cpu); // No Operation

// Table 3-4 CPU Load, Store, and Memory Control Instructions
void lb(R3000 *cpu);  // Load u8
void lbu(R3000 *cpu); // Load u8 Unsigned
void lh(R3000 *cpu);  // Load Halfword
void lhu(R3000 *cpu); // Load Halfword Unsigned
void lw(R3000 *cpu);  // Load Word
void lwl(R3000 *cpu); // Load Word Left
void lwr(R3000 *cpu); // Load Word Right

void sb(R3000 *cpu);  // Store u8
void sh(R3000 *cpu);  // Store Halfword
void sw(R3000 *cpu);  // Store Word
void swl(R3000 *cpu); // Store Word Left
void swr(R3000 *cpu); // Store Word Right

// Table 3-5 CPU Logical Instructions
void and_(R3000 *cpu); // And 
void andi(R3000 *cpu); // And Immediate
void lui(R3000 *cpu);  // Load Upper Immediate
void nor(R3000 *cpu);  // Not Or
void or_(R3000 *cpu);   // Or
void ori(R3000 *cpu);  // Or Immediate
void xor_(R3000 *cpu);  // Exclusive Or
void xori(R3000 *cpu); // Exclusive Or Immediate

// Table 3-7 CPU Move Instructions
void mfhi(R3000 *cpu); // Move From HI Register
void mthi(R3000 *cpu); // Move To HI Register
void mflo(R3000 *cpu); // Move From LO Register
void mtlo(R3000 *cpu); // Move To LO Register


// Table 3-8 CPU Shift Instructions
void sll(R3000 *cpu);  // Shift Word Left Logical
void sllv(R3000 *cpu); // Shift Word Left Logical
void sra(R3000 *cpu);  // Shift Word Right Arithmetic
void srav(R3000 *cpu); // Shift Word Right Arithmetic Variable
void srl(R3000 *cpu);  // Shift Word Right Logical
void srlv(R3000 *cpu); // Shift Word Right Logical Variable


// Table 3-9 CPU Trap Instructions
void syscall(R3000 *cpu); // System Call
void break_(R3000 *cpu);  // Breakpoint


// Table 3-19 Coprocessor Execute Instructions
void cop0(R3000 *cpu);
void mfc0(R3000 *cpu);
void mtc0(R3000 *cpu);
void rfe(R3000 *cpu);


void cop2(R3000 *cpu); // Coprocessor Operation to Coprocessor 2


// Table 3-20 Coprocessor Load and Store Instructions
void lwc2(R3000 *cpu); // Load Word to Coprocessor 2
void swc2(R3000 *cpu); // Store Word from Coprocessor 2


void breakpoint(R3000 *cpu);

/* vAddr: virtual address */
/* DATA: Indicates that access is for DATA */
/* hint: hint that indicates the possible use of the data */


/*

AccessLength Name value Meaning
DOUBLEWORD         7      8 u8s (64 bits)
SEPTIu8          6      7 u8s (56 bits)
SEXTIu8          5      6 u8s (48 bits)
QUINTIu8         4      5 u8s (40 bits)
WORD               3      4 u8s (32 bits)
TRIPLEu8         2      3 u8s (24 bits)
HALFWORD           1      2 u8s (16 bits)
u8               0      1 u8 (8 bits)

*/
