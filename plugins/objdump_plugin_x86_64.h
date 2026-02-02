/**
 * @file objdump_plugin_x86_64.h
 * @brief x86_64 Specialization of objdump_plugin.h
 */
#ifndef OBJDUMP_PLUGIN_X86_64_H
#define OBJDUMP_PLUGIN_X86_64_H

#include "../opcodes/x86_64_dis_instr_info.h"
#include "objdump_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Specialized implementation for x86_64 (64-bit addresses) */
void instruction_callback(struct instr_info *ins);

#ifdef __cplusplus
}
#endif

typedef void (*plugin_cb_x86_64_t)(struct instr_info *);

#endif // OBJDUMP_PLUGIN_X86_64_H