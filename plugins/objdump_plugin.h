/**
 * @file objdump_plugin.h
 * @brief Base interface for the objdump plugin system.
 * 
 * Generic "Base Contract". Each architecture specializes these calls.
 */
#ifndef OBJDUMP_PLUGIN_H
#define OBJDUMP_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

// --- BASE CONTRACT TYPEDEFS ---

/**
 * @brief Generic instruction callback.
 * specializes to: void instruction_callback(struct instr_info *ins)
 */
typedef void (*plugin_cb_t)(void *ins); 

typedef void (*bad_plugin_cb_t)(const char *reason);
typedef void (*file_plugin_cb_t)(const char *filename);
typedef void (*section_plugin_cb_t)(const char *sec_name, const char *arch_name);
typedef void (*label_plugin_cb_t)(const char *label_name, const char *type, unsigned long addr);

// --- LIFECYCLE CALLBACKS ---
void plugin_start(void);
void plugin_cleanup(void);
void plugin_print_stats(void);

#ifdef __cplusplus
}
#endif

#endif // OBJDUMP_PLUGIN_H