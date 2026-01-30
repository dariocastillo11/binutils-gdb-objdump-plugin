/**
 * @file objdump_plugin_arm.h
 * @brief ARM Specialization of objdump_plugin.h
 */
#ifndef OBJDUMP_PLUGIN_ARM_H
#define OBJDUMP_PLUGIN_ARM_H

#include "../opcodes/arm_dis_instr_info.h"
#include "objdump_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Specialized implementation for ARM (replaces void* with concrete struct) */
void instruction_callback(struct instr_info *ins);

#ifdef __cplusplus
}
#endif

typedef void (*plugin_cb_arm_t)(struct instr_info *);

#endif // OBJDUMP_PLUGIN_ARM_H