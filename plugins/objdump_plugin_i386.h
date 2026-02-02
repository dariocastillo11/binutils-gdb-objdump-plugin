/**
 * @file objdump_plugin_i386.h
 * @brief x86 (32-bit) Specialization of objdump_plugin.h
 */
#ifndef OBJDUMP_PLUGIN_I386_H
#define OBJDUMP_PLUGIN_I386_H

#include "../opcodes/i386_dis_instr_info.h"
#include "objdump_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Full detailed implementation for x86_32 */
void instruction_callback(struct instr_info *ins);

#ifdef __cplusplus
}
#endif

typedef void (*plugin_cb_i386_t)(struct instr_info *);

#endif // OBJDUMP_PLUGIN_I386_H