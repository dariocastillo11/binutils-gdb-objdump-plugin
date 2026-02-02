/**
 * instr_info structure definition for MIPS disassembler plugins
 */
#ifndef MIPS_DIS_INFO_H
#define MIPS_DIS_INFO_H

#include <stdint.h>

/**
 * struct minimal instr_info for MIPS architecture.
 */
struct instr_info {
    uint64_t start_pc;           // Instruction address
    char mnemonic[64];           // Mnemonic
    char operands[256];          // Formatted operands
};
#endif // MIPS_DIS_INFO_H
