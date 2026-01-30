/**
 * instr_info_arm structure definition for ARM disassembler plugins
 */
#ifndef ARM_DIS_INFO_H
#define ARM_DIS_INFO_H

#include <stdint.h>

/**
 * struc minimal instr_info for ARM architecture.
 * the fields can be expanded as needed.
 */
struct instr_info {
    uint64_t start_pc;           // Dirección de la instrucción
    char mnemonic[64];           // Mnemónico
    char operands[256];          // Operandos formateados
};
#endif // ARM_DIS_INFO_H
