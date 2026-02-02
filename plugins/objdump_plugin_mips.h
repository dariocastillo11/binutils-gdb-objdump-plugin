/**
 * @file objdump_plugin_mips.h
 * @brief MIPS Specialization of objdump_plugin.h
 */
#ifndef OBJDUMP_PLUGIN_MIPS_H
#define OBJDUMP_PLUGIN_MIPS_H

#include "../opcodes/mips_dis_instr_info.h"
#include "objdump_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Specialized implementation for MIPS (replaces void* with concrete struct) */
void instruction_callback(struct instr_info *ins);

#ifdef __cplusplus
}
#endif

typedef void (*plugin_cb_mips_t)(struct instr_info *);

#endif // OBJDUMP_PLUGIN_MIPS_H
