/**
 * instr_info structure definition for x86_64 disassembler plugins
 */
#ifndef X86_64_DIS_INFO_H
#define X86_64_DIS_INFO_H
#include <stdint.h>
#include <stdbool.h>

struct instr_info {
  uint64_t start_pc;
  char mnemonic[64];
  char operands[256];
  // Puedes agregar aquí los campos específicos de x86_64 si los necesitas
};

#endif // X86_64_DIS_INFO_H
