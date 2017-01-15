#include "../copyright"

#ifndef _sa1_h_
#define _sa1_h_

#include "memmap.h"
#include "cpuexec.h"

typedef struct
{
   pair      P;
   pair      A;
   pair      D;
   pair      S;
   pair      X;
   pair      Y;
   uint16_t  PC;
   uint8_t   PB;
   uint8_t   DB;
} PACKING SSA1Registers;

typedef struct
{
   uint8_t*      Map [MEMMAP_NUM_BLOCKS];
   uint8_t*      WriteMap [MEMMAP_NUM_BLOCKS];
   SOpcodes*     S9xOpcodes;
   uint8_t*      PC;
   uint8_t*      PCBase;
   uint8_t*      BWRAM;
   uint8_t*      PCAtOpcodeStart;
   uint8_t*      WaitAddress;
   uint8_t*      WaitByteAddress1;
   uint8_t*      WaitByteAddress2;
   SSA1Registers Registers;
   int64_t       sum;
   int           arithmetic_op;
   uint32_t      WaitCounter;
   uint32_t      ShiftedPB;
   uint32_t      ShiftedDB;
   uint32_t      Flags;
   int16_t       op1;
   int16_t       op2;
   uint8_t       _Carry;
   uint8_t       _Zero;
   uint8_t       _Negative;
   uint8_t       _Overflow;
   uint8_t       IRQActive;
   uint8_t       VirtualBitmapFormat;
   uint8_t       variable_bit_pos;
   bool          CPUExecuting;
   bool          Executing;
   bool          NMIActive;
   bool          WaitingForInterrupt;
   bool          Waiting;
   bool          overflow;
   bool          in_char_dma;
} PACKING SSA1;

#define SA1CheckZero() (SA1._Zero == 0)
#define SA1CheckCarry() (SA1._Carry)
#define SA1CheckIRQ() (SA1.Registers.PL & IRQ)
#define SA1CheckDecimal() (SA1.Registers.PL & Decimal)
#define SA1CheckIndex() (SA1.Registers.PL & IndexFlag)
#define SA1CheckMemory() (SA1.Registers.PL & MemoryFlag)
#define SA1CheckOverflow() (SA1._Overflow)
#define SA1CheckNegative() (SA1._Negative & 0x80)
#define SA1CheckEmulation() (SA1.Registers.P.W & Emulation)

#define SA1ClearFlags(f) (SA1.Registers.P.W &= ~(f))
#define SA1SetFlags(f)   (SA1.Registers.P.W |=  (f))
#define SA1CheckFlag(f)  (SA1.Registers.PL & (f))

uint8_t S9xSA1GetByte(uint32_t);
uint16_t S9xSA1GetWord(uint32_t);
void S9xSA1SetByte(uint8_t, uint32_t);
void S9xSA1SetWord(uint16_t, uint32_t);
void S9xSA1SetPCBase(uint32_t);
uint8_t S9xGetSA1(uint32_t);
void S9xSetSA1(uint8_t, uint32_t);

extern SOpcodes S9xSA1OpcodesM1X1 [256];
extern SOpcodes S9xSA1OpcodesM1X0 [256];
extern SOpcodes S9xSA1OpcodesM0X1 [256];
extern SOpcodes S9xSA1OpcodesM0X0 [256];
extern SSA1 SA1;

void S9xSA1MainLoop();
void S9xSA1Init();
void S9xFixSA1AfterSnapshotLoad();

#define SNES_IRQ_SOURCE     (1 << 7)
#define TIMER_IRQ_SOURCE    (1 << 6)
#define DMA_IRQ_SOURCE      (1 << 5)

static inline void S9xSA1UnpackStatus(void)
{
   SA1._Zero = (SA1.Registers.PL & Zero) == 0;
   SA1._Negative = (SA1.Registers.PL & Negative);
   SA1._Carry = (SA1.Registers.PL & Carry);
   SA1._Overflow = (SA1.Registers.PL & Overflow) >> 6;
}

static inline void S9xSA1PackStatus(void)
{
   SA1.Registers.PL &= ~(Zero | Negative | Carry | Overflow);
   SA1.Registers.PL |= SA1._Carry | ((SA1._Zero == 0) << 1) |
                       (SA1._Negative & 0x80) | (SA1._Overflow << 6);
}

static inline void S9xSA1FixCycles(void)
{
   if (SA1CheckEmulation())
      SA1.S9xOpcodes = S9xSA1OpcodesM1X1;
   else if (SA1CheckMemory())
   {
      if (SA1CheckIndex())
         SA1.S9xOpcodes = S9xSA1OpcodesM1X1;
      else
         SA1.S9xOpcodes = S9xSA1OpcodesM1X0;
   }
   else
   {
      if (SA1CheckIndex())
         SA1.S9xOpcodes = S9xSA1OpcodesM0X1;
      else
         SA1.S9xOpcodes = S9xSA1OpcodesM0X0;
   }
}
#endif
