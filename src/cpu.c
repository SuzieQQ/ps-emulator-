#include "cpu.h"

void reset_cpu(R3000 *cpu)
{
  cpu = (R3000*)malloc(sizeof(R3000));

/*
Assertion of the Reset signal causes an exception
that transfers control to the special vector at virtual
address 0xbfc0_0000 (The start of the BIOS)
*/
  cpu->pc = 0xbfc00000;
 
  cpu->hi = 0;
 
  cpu->lo = 0;
}

u32 gpr(R3000 *cpu, u8 index) // General Purpose Register
{
   cpu->gpr_reg[0] = 0x00000000;

   return cpu->gpr_reg[index];
}

u32 set_gpr(R3000 *cpu, u8 index, u8 v)
{
  cpu->gpr_reg[0] = 0x00000000;

  return cpu->gpr_reg[index] = v;

}

// 6-bit operation code
u32 opcode(R3000 *cpu)
{
  return (cpu->opcode >> 26) & 0x3f;
}

// 6-bit function field
u32 function(R3000 *cpu)
{
  return (cpu->opcode & 0x3f);
}

// Register source
u32 rs(R3000 *cpu)
{
  return gpr(cpu, (cpu->opcode >> 21) & 0x1f);
}

// Register target
u32 rt(R3000 *cpu)
{
  return gpr(cpu, (cpu->opcode >> 16) & 0x1f);
}

// Register destination
u32 rd(R3000 *cpu)
{
  return gpr(cpu, (cpu->opcode >> 11) & 0x1f);
}

u32 set_rs(R3000 *cpu, u8 value)
{
  return set_gpr(cpu, rs(cpu), value);
}

u32 set_rt(R3000 *cpu, u8 value)
{
  return set_gpr(cpu, rt(cpu), value);
}

u32 set_rd(R3000 *cpu, u8 value)
{
  return set_gpr(cpu, rd(cpu), value);
}

// 5-bit shift amount
u32 shift(R3000 *cpu)
{
  return (cpu->opcode >> 6) & 0x1f;
}

// 16-bit immediate, branch displacement or address
// displacement

u32 imm16(R3000 *cpu)
{
  return (cpu->opcode & 0xffff);
}

// 16-bit immediate, branch displacement or address
// displacement

s32 imm16sign(R3000 *cpu)
{
  return ((s16)cpu->opcode & 0xffff);
}

// Immediate 20
u32 imm20(R3000 *cpu)
{
  return (cpu->opcode & 0xfffff);
}

// Immediate 25
u32 imm25(R3000 *cpu)
{
  return (cpu->opcode & 0x3ffffff);
}

// 26-bit jump target address target
u32 target(R3000 *cpu)
{
  return (cpu->opcode & 0x3ffffff);
}


// The Cop0 controls the exception handling with the use of the Cause register, the EPC register, the Status
// register, the BADV register, and the Context register. A brief description of each follows, after which the rest of the
// Cop0 registers for breakpoint management will be described for the sake of completeness.

void signalException(R3000 *cpu,u8 cause)
{

  if (cause == Interrupt)
  {
    
    cpu->m_cop0_epc = cpu->pc;

  }else
  {

    cpu->m_cop0_epc = cpu->pc_exception;
  
  }

// Additionally, the BREAK opcode can be used to create further breakpoints by patching the executable code. 
// The BREAK opcode uses the same Excode value (09h) in CAUSE register. However, the BREAK opcode jumps 
// to the normal exception handler at 80000080h (not 80000040h).

if (cause == Overflow){}
  
// COP0 Break  80000040h   BFC00140h (Debug Break)
// General     80000080h   BFC00180h (General Interrupts & Exceptions)

// BEV Bootstrap Exception Vector. The value of this bit determines the locations of the exception vectors of the
// processor. If BEV = 1, then the processor is in “Bootstrap” mode, and the exception vectors reside
// in the BIOS ROM. If BEV = 0, then the processor is in normal mode, and the exception vectors reside in RAM
 u32 addr; 

 if(cpu->m_cop0_sr.boot_exception == 0)
 {

  addr = 0x80000080;

 }else if(cpu->m_cop0_sr.boot_exception == 1)
 {

  addr = 0xbfc00180;

 }

u32 _pending =  cpu->m_cop0_cause.interrupt_pending;

cpu->m_cop0_cause.word = 0;

cpu->m_cop0_cause.interrupt_pending = _pending;

cpu->m_cop0_cause.excode = cause; // ACTIVE EXCEPTION

/////////////////////////////////////////////////////////////////////////

// REVERSE RFE FOR STATUS 
cpu->m_cop0_sr.interrupt_disable  = cpu->m_cop0_sr.current_kernel_mode;

cpu->m_cop0_sr.old_kernel   = cpu->m_cop0_sr.prev_kernel;

cpu->m_cop0_sr.prev_interrupt  =  cpu->m_cop0_sr.interrupt_enable;

cpu->m_cop0_sr.interrupt_enable = false;

cpu->m_cop0_sr.current_kernel_mode = KERNEL;


// Branch Delay. The Branch Delay bit is set (1) if the last exception was taken while the
// processor was executing in the branch delay slot. If so, then the EPC will be rolled back to point to the branch
// instruction, so that it can be re-executed and the branch direction re-determined

  if(cpu->branch_delay_slot_saved)
  {
    cpu->m_cop0_epc -= 4;

    cpu->m_cop0_cause.branch_delay_slot   = true;
   
   if(cpu->branch_taken_saved)
   {
      cpu->m_cop0_cause.branch_delay_slot = true;
   }
 
   cpu->m_cop0_jmptest = cpu->pc; // cop0r6 - JUMPDEST - Randomly memorized jump address (R)

  }
  
  set_pc(cpu,addr);

}

/*
Contains the address whose reference caused an exception. 
Set on any MMU type of exceptions, on references outside of kuseg (in User mode) and on any misaligned reference. 
BadVaddr is updated ONLY by Address errors (Excode 04h and 05h), all other exceptions 
(including bus errors) leave BadVaddr unchanged.
*/

void load_update_badvaddr(R3000 *cpu,u32 addr)
{
    cpu->m_cop0_badvaddr = addr;

    signalException(cpu,LoadAddrError);
}
void write_update_badvaddr(R3000 *cpu,u32 addr)
{
    cpu->m_cop0_badvaddr = addr;
    
    signalException(cpu,WriteAddrError);

}


void set_pc(R3000 *cpu,u32 addr) 
{

  cpu->pc = addr;
  
  cpu->next_pc = addr + 4;

}

void jump_addr(R3000 *cpu,u32 addr)
{
  cpu->next_pc = addr;
}

void JumpDelaySlot(R3000 *cpu,u32 reg,u32 value)
{
  if(reg == 0)
    return;

    cpu->slot_next.reg = reg;           // old

    cpu->slot_next.cur = value;         // current

    cpu->slot_next.prev = gpr(cpu,reg); // previous
 
}


// Table 3-1 CPU Arithmetic Instructions

void add(R3000 *cpu) // Add Word
{

  u32 res = rs(cpu) + rt(cpu);

  if (~((rs(cpu) ^ rt(cpu)) & (rs(cpu) ^ res) & 0x80000000) == 0)
  {
    signalException(cpu,Overflow);
    return;
  }
  else
  {
    set_rd(cpu, res);
  }
}

void addi(R3000 *cpu) // Add Immediate Word
{
  u32 res = rs(cpu) + imm16sign(cpu);

  if (~((rs(cpu) ^ rt(cpu)) & (rs(cpu) ^ res) & 0x80000000) == 0)
  {
    signalException(cpu,Overflow);
    return;
  }
  else
  {
    set_rt(cpu, res);
  }
}

void addiu(R3000 *cpu) // // Add Immediate Unsigned Word
{
  u32 res = rs(cpu) + imm16sign(cpu);
  set_rt(cpu, res);
}

void addu(R3000 *cpu) // Add Unsigned Word
{
  u32 res = rs(cpu) + rt(cpu);
  set_rd(cpu, res);
}

void div_(R3000 *cpu) // Divide Word
{

  s32 sign_rs = (s32)rs(cpu);

  s32 sign_rt = (s32)rt(cpu);

  if (sign_rt == 0)
  {
    cpu->lo = (sign_rs < 0) ? 0x00000001 : 0xffffffff;

    cpu->hi = sign_rs;
  }
  else if ((u32)sign_rt == 0x80000000 && (u32)sign_rt == 0xffffffff)
  {

    cpu->lo = 0x80000000;

    cpu->hi = 0x00000000;
  }
  else
  {

    cpu->lo = sign_rs / sign_rt; // div 2’s complement integer division

    cpu->hi = sign_rs % sign_rt; // mod  2’s complement modulo
  }
}

void divu(R3000 *cpu) // Divide Unsigned Word
{

  u32 usign_rs = rs(cpu);

  u32 usign_rt = rt(cpu);

  if (usign_rt == 0)
  {
    cpu->lo = 0xffffffff;

    cpu->hi = usign_rs;
  }
  else
  {

    cpu->lo = usign_rs / usign_rt; // div 2’s complement integer division

    cpu->hi = usign_rs % usign_rt; // mod  2’s complement modulo
  }
}

void mult(R3000 *cpu) // Multiply Word to GPR
{
  u64 res = (s64)(s32)rs(cpu) * (s64)(s32)rt(cpu);
  
  cpu->lo = (res & 0xffffffff);
  cpu->hi = (res >> 32);
}

void multu(R3000 *cpu) // Multiply Unsigned Word
{
  u64 res = (u64)rs(cpu) * (u64)rt(cpu);

  cpu->lo = (res & 0xffffffff);
  cpu->hi = (res >> 32);
}
// pagina 34

void slt(R3000 *cpu) // Set on Less Than
{
  s32 sign_rs = rs(cpu);
  s32 sign_rt = rt(cpu);

  if (sign_rs < sign_rt)
  {
    set_rd(cpu, 1);
  }
  else
  {
    set_rd(cpu, 0);
  }
}

void slti(R3000 *cpu) // Set on Less Than Immediate
{

  if ((s32)rs(cpu) < (s16)imm16sign(cpu))
  {
    set_rt(cpu, 1);
  }
  else
  {
    set_rt(cpu, 0);
  }
}

void sltiu(R3000 *cpu) // Set on Less Than Immediate Unsigned
{
  if (rs(cpu) < (u16)imm16sign(cpu))
  {
    set_rt(cpu, 1);
  }
  else
  {
    set_rt(cpu, 0);
  }
}
void sltu(R3000 *cpu) // Set on Less Than Unsigned
{
  u32 usign_rs = rs(cpu);
  u32 usign_rt = rt(cpu);

  if (usign_rs < usign_rt)
  {
    set_rd(cpu, 1);
  }
  else
  {
    set_rd(cpu, 0);
  }
}

void sub(R3000 *cpu) // Subtract Word
{
  u32 res = rs(cpu) - rt(cpu);

  if (~((rs(cpu) ^ rt(cpu)) & (rs(cpu) ^ res) & 0x80000000) == 0)
  {
    signalException(cpu,Overflow);
    return;
  }
  else
  {
    set_rd(cpu, res);
  }
}

void subu(R3000 *cpu) // Subtract Unsigned Word
{
  u32 res = rs(cpu) - rt(cpu);
  set_rd(cpu, res);
}


void bcondz(R3000 *cpu) // Unconditional Branch
{

  cpu->delay_slot = true;

  bool target = rt(cpu) & 0x01;

  s16 offset = (imm16sign(cpu) << 2);
 
  bool condition;
  
  if(target)
  {
    // BGTZ
    condition = (s32)rs(cpu) >= 0; 
  }else
  {
    // BLTZ
    condition = (s32)rs(cpu) < 0; 
  }

  // BLTZAL
  if(!condition)
  set_gpr(cpu,31,cpu->next_pc);
  

  // BLTZAL
  if(condition)
  set_gpr(cpu,31,cpu->next_pc);
  

 if(condition){
  jump_addr(cpu,(cpu->pc + offset));
 }


}

// Table 3-2 CPU Branch and Jump Instructions
void beq(R3000 *cpu) // Branch on Equal
{
  s16 offset = (imm16sign(cpu) << 2);
  
  cpu->delay_slot = true;
  
  if(rs(cpu) == rt(cpu))
  {
 
    jump_addr(cpu,(cpu->pc + offset));
 
  }

}

void bgtz(R3000 *cpu) // Branch on Greater Than or Equal to Zero
{
  s16 offset = (imm16sign(cpu) << 2);
   
  cpu->delay_slot = true;

  if((s32)rs(cpu) > 0)
  {
 
    jump_addr(cpu,(cpu->pc + offset));
  
  }

}

void blez(R3000 *cpu) // Branch on Less Than or Equal to Zero
{
  s16 offset = (imm16sign(cpu) << 2);
   
  cpu->delay_slot = true;

  if((s32)rs(cpu) <= 0)
  {
  
    jump_addr(cpu,(cpu->pc + offset));
 
  }

}

void bne(R3000 *cpu) // Branch on Not Equal
{
  s16 offset = (imm16sign(cpu) << 2);
   
  cpu->delay_slot = true;

  if(rs(cpu) != rt(cpu))
  {
  
    jump_addr(cpu,(cpu->pc + offset));
  
  }
}

void j(R3000 *cpu) // Jump
{
  cpu->delay_slot = true;

  jump_addr(cpu, (cpu->next_pc & 0xf0000000) | (target(cpu) << 2));
}

void jal(R3000 *cpu) // Jump and Link
{
  cpu->delay_slot = true;
    
  set_gpr(cpu,31,cpu->next_pc); 

  jump_addr(cpu, (cpu->next_pc & 0xf0000000) | (target(cpu) << 2));

}

void jalr(R3000 *cpu) // Jump and Link Register
{}

void jr(R3000 *cpu) // Jump Register
{
    cpu->delay_slot = true;
    
    const u32 addr = rs(cpu);
  
}
// Table 3-3 CPU Instruction Control Instructions

void nop(R3000 *cpu) // No Operation
{}


// Table 3-4 CPU Load, Store, and Memory Control Instructions

void lb(R3000 *cpu) // Load byte
{
   set_rt(cpu,(s8)read8(rs(cpu) + imm16sign(cpu)));
}

void lbu(R3000 *cpu) // Load byte Unsigned
{
  set_rt(cpu,read8(rs(cpu) + imm16sign(cpu)));
}

void lh(R3000 *cpu) // Load Halfword
{
  set_rt(cpu,(s16)read16(rs(cpu) + imm16sign(cpu)));
}

void lhu(R3000 *cpu) // Load Halfword Unsigned
{
  set_rt(cpu,read16(rs(cpu) + imm16sign(cpu)));
}

void lw(R3000 *cpu) // Load Word
{
  set_rt(cpu,(s32)read32(rs(cpu) + imm16sign(cpu)));
}

void lwl(R3000 *cpu) // Load Word Left
{
  u32 addr = (rs(cpu) + imm16sign(cpu)); // (EffAddr)
  
  u32 mem = read32(addr & ~0x3);
  
  u32 aligned_addr = addr & 0x3; 

  u32 val;

  switch (aligned_addr) // load aligned addr
  {
  case 0: val = (rt(cpu) & 0x00ffffff) | (mem << 24); break;
  case 1: val = (rt(cpu) & 0x0000ffff) | (mem << 16); break;
  case 2: val = (rt(cpu) & 0x000000ff) | (mem << 8);  break;
  case 3: val = (rt(cpu) & 0x00000000) |  mem; break;
  }
  
  set_rt(cpu,val);

}

void lwr(R3000 *cpu) // Load Word Right
{
  u32 addr = (rs(cpu) + imm16sign(cpu)); // (EffAddr)
  
  u32 mem = read32(addr & ~0x3);
  
  u32 aligned_addr = (addr & 0x3); 

  u32 val;
 
  switch (aligned_addr)
  {
  
  case 0: val = (rt(cpu) & 0x00000000) | (mem);       break;
  case 1: val = (rt(cpu) & 0xff000000) | (mem >> 8);  break;
  case 2: val = (rt(cpu) & 0xffff0000) | (mem >> 16); break;
  case 3: val = (rt(cpu) & 0xffffff00) | (mem >> 24); break;
  }
  
  set_rt(cpu,val);

}

void sb(R3000 *cpu)  // To store a byte to memory
{
  write8(rs(cpu) + imm16sign(cpu),(rt(cpu) & 0xff));
}

void sh(R3000 *cpu)  // Store Halfword
{
  
  write16(rs(cpu) + imm16sign(cpu),(rt(cpu) & 0xffff));

}

void sw(R3000 *cpu)  // Store Word
{

  write32(rs(cpu) + imm16sign(cpu),rt(cpu));

}

void swl(R3000 *cpu) // Store Word Left
{
  u32 addr = (rs(cpu) + imm16sign(cpu)); // (EffAddr)
  
  u32 mem = read32(addr & ~0x3);
  
  u32 aligned_addr = addr & 0x3; 

  u32 val;

  switch (aligned_addr) // load aligned addr
  {
  case 0: val = (mem & 0xffffff00) | rt(cpu) >> 24; break;
  case 1: val = (mem & 0xffff0000) | rt(cpu) >> 16; break;
  case 2: val = (mem & 0xff000000) | rt(cpu) >> 8; break;
  case 3: val = (mem & 0x00000000) | rt(cpu); break;
  }
  
  write32(addr & ~0x3 ,val);

}

void swr(R3000 *cpu) // Store Word Right
{
  u32 addr = (rs(cpu) + imm16sign(cpu)); // (EffAddr)
  
  u32 mem = read32(addr & ~0x3);
  
  u32 aligned_addr = addr & 0x3; 

  u32 val;

  switch (aligned_addr) // load aligned addr
  {

  case 0: val = (mem & 0x00000000) | rt(cpu);       break;
  case 1: val = (mem & 0x000000ff) | rt(cpu) << 8;  break;
  case 2: val = (mem & 0x0000ffff) | rt(cpu) << 16; break;
  case 3: val = (mem & 0x00ffffff) | rt(cpu) << 24; break;
  }
  
  write32(addr & ~0x3,val);
}


// Table 3-5 CPU Logical Instructions
void and_(R3000 *cpu) // And
{
  u32 res = rs(cpu) & rt(cpu);
  set_rd(cpu, res);
}

void andi(R3000 *cpu) // And Immediate
{
  u32 res = rs(cpu) & imm16(cpu);
  set_rt(cpu, res);
}

void lui(R3000 *cpu) // Load Upper Immediate
{
  u16 res = imm16(cpu) << 16;
  set_rt(cpu, res);
}

void nor(R3000 *cpu) // Not Or
{
  u32 res = ~rs(cpu) | rt(cpu);
  set_rd(cpu, res);
}

void or_(R3000 * cpu) // Or
{
  u32 res = rs(cpu) | rt(cpu);
  set_rd(cpu, res);
}

void ori(R3000 *cpu) // Or Immediate
{
  u32 res = rs(cpu) | imm16(cpu);
  set_rt(cpu, res);
}

void xor_(R3000 * cpu) // Exclusive Or
{
  u32 res = rs(cpu) ^ rt(cpu);
  set_rd(cpu, res);
}

void xori(R3000 *cpu) // Exclusive Or Immediate
{
  u32 res = rs(cpu) ^ imm16(cpu);
  set_rt(cpu, res);
}

// Table 3-7 CPU Move Instructions
void mfhi(R3000 *cpu) // Move From HI Register
{
  set_rd(cpu, cpu->hi);
}

void mthi(R3000 *cpu) // Move To HI Register
{
  cpu->hi = rs(cpu);
}

void mflo(R3000 *cpu) // Move From LO Register
{
  set_rd(cpu, cpu->lo);
}

void mtlo(R3000 *cpu) // Move To LO Register
{
  cpu->lo = rs(cpu);
}

// Table 3-8 CPU Shift Instructions
void sll(R3000 *cpu) // Shift Word Left Logical
{

  set_rd(cpu, rt(cpu) << shift(cpu));

}

void sllv(R3000 *cpu) // Shift Word Left Logical
{

  set_rd(cpu, rt(cpu) << (rs(cpu) & 0x1f));

}

void sra(R3000 *cpu) // Shift Word Right Arithmetic
{

  set_rd(cpu, (s32)rt(cpu) >> shift(cpu));

}

void srav(R3000 *cpu) // Shift Word Right Arithmetic Variable
{

  set_rd(cpu, (s32)rt(cpu) >> (rs(cpu) & 0x1f));

}

void srl(R3000 *cpu) // Shift Word Right Logical
{

  set_rd(cpu, rt(cpu) >> shift(cpu));

}

void srlv(R3000 *cpu) // Shift Word Right Logical Variable
{

  set_rd(cpu, rt(cpu) >> (rs(cpu) & 0x1f));

}


// Table 3-9 CPU Trap Instructions
void syscall(R3000 *cpu)
{ 
  u32  funcnumb = gpr(cpu,4);
  // There are 3 more i know that arent called the same way as above:
  switch (funcnumb){
  case 0: printf("Exception(){li $a0,0 syscall }");            break;
  case 1: printf("EnterCriticalSection(){li $a0,1 syscall }"); break;
  case 2: printf("ExitCriticalSection(){li $a0,2 syscall }");  break;
  default:
    break;
  }
  
  signalException(cpu,Syscall);

}

void break_(R3000 *cpu)
{

//   COP0 Break    80000040h     BFC00140h   (Debug Break)
  
  if (BreakPoint)
  {
      
    cpu->m_cop0_dcic  |= 1;
    
    set_pc(cpu,0x80000040);

  }
  

}

// Table 3-19 Coprocessor Execute Instructions


void mfc0(R3000 *cpu)
{

switch (rd(cpu))
{
case BPC:      JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_bpc);             break;
case BDA:      JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_bda);             break;
case JUMPDEST: JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_jmptest);         break;
case DCIC:     JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_dcic);            break;
case BadVaddr: JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_badvaddr);        break; 
case BDAM:     JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_bdam);            break;    
case BPCM:     JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_bpcm);            break;     
case SR:       JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_sr.word);         break;   
case CAUSE:    JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_cause.word);      break;    
case EPC:      JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_epc);             break;   
case PRID:     JumpDelaySlot(cpu,rt(cpu),cpu->m_cop0_prid);            break;    
default: printf("unhandled cop0  write %d \n",rd(cpu)); break;
}


}

// Instruction	Format and Description	
// Move To CP0	MTC0 rt, rd Store contents of CPU register rt into register rd of CP0. This follows the convention of store operations.	
// Move From CP0	MFC0 rt, rd Load CPU register rt with contents of CP0 register rd.	
// Restore From Exception	RFE Restore previous interrupt mask and mode bits of status register into current status bits. Restore old status bits into previous status bits.	


void mtc0(R3000 *cpu)
{

switch (rd(cpu)) {
case BPC:      cpu->m_cop0_bpc     = rt(cpu);       break;
case BDA:      cpu->m_cop0_bda     = rt(cpu);       break;
case JUMPDEST: break; // Read Only        
case DCIC:     cpu->m_cop0_dcic    = rt(cpu);       break;
case BadVaddr: break; // Read Only 
case BDAM:     cpu->m_cop0_bdam    = rt(cpu);       break;    
case BPCM:     cpu->m_cop0_bpcm    = rt(cpu);       break;     
case SR:       cpu->m_cop0_sr.word = rt(cpu);       break;   
case CAUSE:    cpu->m_cop0_cause.word  = cpu->m_cop0_cause.word >> 8 & ~0x1ff | rt(cpu) >> 8 & 0x1ff; break; // Read Only    
case EPC:      break; // Read Only 
case PRID:     break; // Read Only   
default: printf("unhandled cop0 read %d \n",rd(cpu)); break;

}


}

// Restore From Exception	RFE Restore previous interrupt mask and mode bits of status register into current status bits
// The RFE opcode moves some bits in cop0r12 (SR): bit2-3 are copied to bit0-1, and bit4-5 are copied to bit2-3, all other bits (including bit4-5) are left unchanged.
// The RFE opcode does NOT automatically jump to EPC. Instead, the exception handler must copy EPC into a register (usually R26 aka K0), and then jump to that address. Because of branch delays, that would look like so:
void rfe(R3000 *cpu)
{
cpu->m_cop0_sr.interrupt_enable    = cpu->m_cop0_sr.prev_interrupt;

cpu->m_cop0_sr.current_kernel_mode = cpu->m_cop0_sr.prev_kernel;

/////////////////////////////////////////////////////////////////////////

cpu->m_cop0_sr.prev_interrupt   =  cpu->m_cop0_sr.interrupt_disable;

cpu->m_cop0_sr.prev_kernel      = cpu->m_cop0_sr.old_kernel;


}



void cop0(R3000 *cpu) 
{

// System Control Co-processor (COP0) Instructions

/*
Coprocessor Opcode/Parameter Encoding

  31..26 |25..21|20..16|15..11|10..6 |  5..0  |
   6bit  | 5bit | 5bit | 5bit | 5bit |  6bit  |
  -------+------+------+------+------+--------+------------
  0100nn |0|0000| rt   | rd   | N/A  | 000000 | MFCn rt,rd_dat  ;rt = dat
  0100nn |0|0010| rt   | rd   | N/A  | 000000 | CFCn rt,rd_cnt  ;rt = cnt
  0100nn |0|0100| rt   | rd   | N/A  | 000000 | MTCn rt,rd_dat  ;dat = rt
  0100nn |0|0110| rt   | rd   | N/A  | 000000 | CTCn rt,rd_cnt  ;cnt = rt
  0100nn |0|1000|00000 | <--immediate16bit--> | BCnF target ;jump if false
  0100nn |0|1000|00001 | <--immediate16bit--> | BCnT target ;jump if true
  0100nn |1| <--------immediate25bit--------> | COPn imm25
  010000 |1|0000| N/A  | N/A  | N/A  | 000001 | COP0 01h  ;=TLBR, unused on PS1
  010000 |1|0000| N/A  | N/A  | N/A  | 000010 | COP0 02h  ;=TLBWI, unused on PS1
  010000 |1|0000| N/A  | N/A  | N/A  | 000110 | COP0 06h  ;=TLBWR, unused on PS1
  010000 |1|0000| N/A  | N/A  | N/A  | 001000 | COP0 08h  ;=TLBP, unused on PS1
  010000 |1|0000| N/A  | N/A  | N/A  | 010000 | COP0 10h  ;=RFE
  1100nn | rs   | rt   | <--immediate16bit--> | LWCn rt_dat,[rs+imm]
  1110nn | rs   | rt   | <--immediate16bit--> | SWCn rt_dat,[rs+imm]
*/


switch (rs(cpu))
{
case 0b0000:  mfc0(cpu); break;
case 0b100:   mtc0(cpu); break;
case 0b10000: rfe(cpu); break;

default:
  break;
}
  
  

}

// COP2 Geometry Transformation Engine (GTE) - 64 registers (most are used)

void cop2(R3000 *cpu) // Coprocessor Operation to Coprocessor 2
{
 // gte not implemented  
}


// Table 3-20 Coprocessor Load and Store Instructions
void lwc2(R3000 *cpu) // Load Word to Coprocessor 2
{  
  u32 addr = rs(cpu) + imm16sign(cpu);
  
  u32 mem = read32(addr); 

  u32 reg_dest = rt(cpu);


  
}

void swc2(R3000 *cpu) // Store Word from Coprocessor 2
{  
  u32 addr = (rs(cpu) + imm16sign(cpu));

}


void execute_cpu(R3000 *cpu)
{

  switch (opcode(cpu))
  {
  case 0x00: special(cpu);  break; //   00h=SPECIAL
  case 0x01: bcondz(cpu); break; //   01h=BcondZ
  case 0x02: j(cpu);     break; //   02h=J
  case 0x03: jal(cpu);   break; //   03h=JAL
  case 0x04: beq(cpu);   break; // 04h=BEQ
  case 0x05: bne(cpu);   break; // 05h=BNE
  case 0x06: blez(cpu);  break; // 06h=BLEZ
  case 0x07: bgtz(cpu);  break; // 07h=BGTZ
  case 0x08: addi(cpu);  break; // 08h=ADDI
  case 0x09: addiu(cpu); break; // 09h=ADDIU
  case 0x0a: slti(cpu);  break; // 0Ah=SLTI
  case 0x0b: sltiu(cpu); break; // 0Bh=SLTIU
  case 0x0c: andi(cpu);  break; // 0Ch=ANDI
  case 0x0d: ori(cpu);   break; // 0Dh=ORI
  case 0x0e: xori(cpu);  break; // 0Eh=XORI
  case 0x0f: lui(cpu);   break; // 0Fh=LUI
  case 0x10: cop0(cpu);  break; // 10h=COP0
  // case 0b1000000000: mfc0(cpu); break; // mfc0
  // case 0b1000000100: mtc0(cpu); break; // mtc0
  // case 0b1000010000: rfe(cpu);  break; // rfe
  case 0x11: break; //   11h=COP1
  case 0x12: cop2(cpu);  break; // 12h=COP2
  case 0x13: break; // 13h=COP3
  case 0x14: break; // 14h=N/A
  case 0x15: break; // 15h=N/A
  case 0x16: break; // 16h=N/A
  case 0x17: break; // 17h=N/A
  case 0x18: break; // 18h=N/A
  case 0x19: break; // 19h=N/A
  case 0x1a: break; // 1Ah=N/A 
  case 0x1b: break; // 1Bh=N/A 
  case 0x1c: break; // 1Ch=N/A 
  case 0x1d: break; // 1Dh=N/A 
  case 0x1e: break; // 1Eh=N/A 
  case 0x1f: break; // 1Fh=N/A 

  case 0x20: lb(cpu);  break; // 20h=LB 
  case 0x21: lh(cpu);  break; // 21h=LH 
  case 0x22: lwl(cpu); break; // 22h=LWL 
  case 0x23: lw(cpu);  break; // 23h=LW 
  case 0x24: lbu(cpu); break; // 24h=LBU
  case 0x25: lhu(cpu); break; // 25h=LHU
  case 0x26: lwr(cpu); break; // 26h=LWR
  case 0x27: break; // 27h=N/A
  case 0x28: sb(cpu); break;   // 28h=SB
  case 0x29: sh(cpu); break;   // 29h=SH
  case 0x2a: swl(cpu); break;  // 2Ah=SWL
  case 0x2b: sw(cpu); break;   // 2Bh=SW 
  case 0x2c: break;  // 2Ch=N/A
  case 0x2d: break;  // 2Dh=N/A
  case 0x2e: swr(cpu); break;  // 2Eh=SWR
  case 0x2f: break;  // 2Fh=N/0
  case 0x30: break; // 30h=LWC0
  case 0x31: break; // 31h=LWC1
  case 0x32: lwc2(cpu); break; // 32h=LWC2 
  case 0x33: break; // 33h=LWC3
  case 0x34: break; // 34h=N/A 
  case 0x35: break; // 35h=N/A 
  case 0x36: break; // 36h=N/A 
  case 0x37: break; // 37h=N/A 
  case 0x38: break; // 38h=SWC0
  case 0x39: break; // 39h=SWC1
  case 0x3a:swc2(cpu); break; // 3Ah=SWC2
  case 0x3b: break; // 3Bh=SWC3
  case 0x3c: break; // 3ch=N/A
  case 0x3d: break; // 3Dh=N/A
  case 0x3e: break; // 3eh=N/A
  case 0x3f: break; // 3fh=N/A
  
  default: printf("error opcode 0x%02.x",opcode(cpu));
    break;
  }
}

void special(R3000 *cpu)
{
  switch (function(cpu))
  {
  case 0x00:sll(cpu);  break; // 00h=SLL
  case 0x01: break; // 01h=N/A
  case 0x02:srl(cpu);  break; // 02h=SRL
  case 0x03:sra(cpu);  break; // 03h=SRA
  case 0x04:sllv(cpu); break; // 04h=SLLV
  case 0x05: break; // 05h=N/A 
  case 0x06:srlv(cpu); break; // 06h=SRLV
  case 0x07:srav(cpu); break; // 07h=SRAV
  case 0x08:jr(cpu);   break; // 08h=JR
  case 0x09:jalr(cpu); break; // 09h=JALR 
  case 0x0a: break; // 0Ah=N/A  
  case 0x0b: break; // 0Bh=N/A 
  case 0x0c:syscall(cpu); break; // 0Ch=SYSCALL
  case 0x0d:break_(cpu); break; // 0Dh=BREAK
  case 0x0e: break; // 0Eh=N/A
  case 0x0f: break; // 0Fh=N/A
  case 0x10:mfhi(cpu); break; // 10h=MFHI
  case 0x11:mthi(cpu); break; // 11h=MTHI 
  case 0x12:mflo(cpu); break; // 12h=MFLO
  case 0x13:mtlo(cpu); break; // 13h=MTLO
  case 0x14: break; // 14h=N/A
  case 0x15: break; // 15h=N/A
  case 0x16: break; // 16h=N/A
  case 0x17: break; // 17h=N/A
  case 0x18:mult(cpu);  break; // 18h=MULT
  case 0x19:multu(cpu); break; // 19h=MULTU 
  case 0x1a:div_(cpu);   break; // 1Ah=DIV
  case 0x1b:divu(cpu);  break; // 1Bh=DIVU
  case 0x1c: break; // 1Ch=N/A
  case 0x1d: break; // 1Dh=N/A
  case 0x1e: break; // 1Eh=N/A
  case 0x1f: break; // 1Fh=N/A
  case 0x20:add(cpu);  break; // 20h=ADD  
  case 0x21:addu(cpu); break; // 21h=ADDU 
  case 0x22:sub(cpu);  break; // 22h=SUB  
  case 0x23:subu(cpu); break; // 23h=SUBU 
  case 0x24:and_(cpu); break; // 24h=AND  
  case 0x25:or_(cpu);   break; // 25h=OR   
  case 0x26:xor_(cpu);  break; // 26h=XOR  
  case 0x27:nor(cpu);  break; // 27h=NOR  
  case 0x28: break; // 28h=N/A  
  case 0x29: break; // 29h=N/A  
  case 0x2a:slt(cpu);  break; // 2Ah=SLT  
  case 0x2b:sltu(cpu); break; // 2Bh=SLTU 
  case 0x2c: break; // 2Ch=N/A  
  case 0x2d: break; // 2Dh=N/A  
  case 0x2e: break; // 2Eh=N/A  
  case 0x2f: break; // 2Fh=N/A  
  case 0x30: break; // 30h=N/A  
  case 0x31: break; // 31h=N/A
  case 0x32: break; // 32h=N/A
  case 0x33: break; // 33h=N/A
  case 0x34: break; // 34h=N/A
  case 0x35: break; // 35h=N/A
  case 0x36: break; // 36h=N/A
  case 0x37: break; // 37h=N/A      
  case 0x38: break; // 38h=N/A
  case 0x39: break; // 39h=N/A
  case 0x3a: break; // 3Ah=N/A
  case 0x3b: break; // 3Bh=N/A   
  case 0x3c: break; // 3Ch=N/A
  case 0x3d: break; // 3Dh=N/A
  case 0x3e: break; // 3Eh=N/A
  case 0x3f: break; // 3Fh=N/A     

  default:printf("error function 0x%02.x",function(cpu));
    break;
  }
}

char *namereg(u8 index)
{
  char *string = malloc(sizeof(char) * 100);

  switch (index)
  {
  case 0:
    string = "R0 zero Constant (always 0";
    break;
  case 1:
    string = "R1 at Assembler temporary (destroyed by some assembler pseudoinstructions!)";
    break;
  case 2:
    string = "R2 v0 Subroutine return values, may be changed by subroutines";
    break;
  case 3:
    string = "R3 v1 Subroutine return values, may be changed by subroutines";
    break;
  case 4:
    string = "R4 a0 Subroutine arguments, may be changed by subroutines";
    break;
  case 5:
    string = "R5 a1 Subroutine arguments, may be changed by subroutines";
    break;
  case 6:
    string = "R6 a2 Subroutine arguments, may be changed by subroutines";
    break;
  case 7:
    string = "R7 a3 Subroutine arguments, may be changed by subroutines";
    break;
  case 8:
    string = "R8 t0 Temporaries, may be changed by subroutines";
    break;
  case 9:
    string = "R9 t1 Temporaries, may be changed by subroutines";
    break;
  case 10:
    string = "R10 t2 Temporaries, may be changed by subroutines";
    break;
  case 11:
    string = "R11 t3 Temporaries, may be changed by subroutines";
    break;
  case 12:
    string = "R12 t4 Temporaries, may be changed by subroutines";
    break;
  case 13:
    string = "R13 t5 Temporaries, may be changed by subroutines";
    break;
  case 14:
    string = "R14 t6 Temporaries, may be changed by subroutines";
    break;
  default:
    break;
  }

  return string;
}