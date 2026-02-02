#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>
#include <fstream>
#include <string_view>
#include <re2/re2.h>
#include "objdump_plugin.h"
// Then include the architecture-specific headers depending on the build target
#if defined(TARGET_I386) 
#include "../opcodes/i386_dis_instr_info.h"
#include "objdump_plugin_i386.h"
#elif defined(TARGET_X86_64) 
#include "../opcodes/x86_64_dis_instr_info.h"
#include "objdump_plugin_x86_64.h"
#elif defined(TARGET_ARM) 
#include "../opcodes/arm_dis_instr_info.h"
#include "objdump_plugin_arm.h"
#elif defined(TARGET_MIPS) 
#include "../opcodes/mips_dis_instr_info.h"
#include "objdump_plugin_mips.h"
#endif

// Statistical counters
static unsigned long insn_count = 0;
static unsigned long file_count = 0;
static unsigned long section_count = 0;
static unsigned long label_count = 0;
static unsigned long bad_count = 0;
static unsigned long call_count = 0;
static unsigned long jump_count = 0;
static unsigned long return_count = 0;
static unsigned long lock_count = 0;
static unsigned long rep_count = 0;
static unsigned long canary_count = 0;
static unsigned long rex_count = 0;

// Regex pattern counters
static unsigned long pattern_matches = 0;  // DEREFERENCE_PATTERN
static unsigned long pattern_matches2 = 0;  // INDIRECT_CALL_NO_OFFSET_PATTERN

// Pre-compiled RE2 patterns (faster than std::regex)
static RE2 *pattern1 = nullptr;
static RE2 *pattern2 = nullptr;

// string for operands. Reserve size.
static std::string operandString;
static std::string nmemonicString;

static std::string g_current_arch = "Generic";

// Function prototypes
void write_instruction_to_file(const struct instr_info *ins, const std::string& operandString);
void count_instruction_types(const struct instr_info *ins);
void print_instruction_console(const struct instr_info *ins);
void analyze_prefixes(const struct instr_info *ins);
void print_operands(std::string operands);
 //void detect_regex_patterns(const char* operands);
 void detect_regex_patterns(std::string operands);

// Output file
static FILE *output_file = NULL;

/**
 * Initialization function called at plugin load (after dlopen)
 */
extern "C" void plugin_start(void) {
    operandString.reserve(40);
    nmemonicString.reserve(16);
  
    // Open output file for writing using C FILE
    output_file = fopen("disasm_output.txt", "w");
    if (output_file == NULL) {
        std::cerr << "Error: Could not open output file\n";
        return;
    }
    try {
        pattern1 = new RE2(R"((\*)?("?[&]?[a-zA-Z0-9_-]+"?|0x[0-9a-fA-F]+|-0x[0-9a-fA-F]+)?[\(\[][^)\]]+[\)\]])");
        pattern2 = new RE2(R"(^\*(%[a-z0-9]+)$)");
    } catch (const std::exception& e) {
        std::cerr << "Error: Could not compile regex patterns\n";
    }
}

/**
 * Cleanup function called at plugin unload (before dlclose)
 */
extern "C" void plugin_cleanup(void) {
   // Close output file
   if (output_file != NULL) {
       fprintf(output_file, "\n");
       fclose(output_file);
       output_file = NULL;
   }
   // Clean up regex patterns
   delete pattern1;
   pattern1 = nullptr;
  
   delete pattern2;
   pattern2 = nullptr;
}

/**
 * Handle an instruction callback, incrementing the instruction counter and processing the instruction.
 */
extern "C" void instruction_callback(struct instr_info *ins) {
    if (!ins) {
        return;
    }
    insn_count++;
    operandString.clear();
    operandString.assign(ins->operands);
    operandString.erase(std::remove(operandString.begin(), operandString.end(), ' '), operandString.end());

    nmemonicString.clear();
    nmemonicString.assign(ins->mnemonic);

    write_instruction_to_file(ins, operandString);
    count_instruction_types(ins);
    
    print_instruction_console(ins);
    analyze_prefixes(ins);
    
    detect_regex_patterns(operandString);
}

/**
 * Handle a bad instruction callback, incrementing the bad instruction counter and printing the reason.
 */
extern "C" void bad_plugin_callback(const char *bad_reason) {
    bad_count++;
    std::cout << "[BAD] " << bad_reason << "\n";
}

/**
 * Handle a new file callback, incrementing the file counter and printing the filename.
 */
extern "C" void file_plugin_callback(const char *filename) {
    file_count++;
    if (!filename) {
        return;
    }
    std::cout << "=== NEW FILE: " << filename << " ===\n\n\n";
}

/**
 * Handle a new section callback, incrementing the section counter and printing the section name and architecture.
 */
extern "C" void section_plugin_callback(const char *section_name, const char *arch_name) {
    if (section_name) {
        section_count++;
        if (arch_name) g_current_arch = arch_name;
        std::cout << ">>> SECTION: " << section_name << " (Arch: " << arch_name << ")\n\n\n";
    }
}

/**
 * Handle a new label callback, incrementing the label counter and printing the label name, type, and address.
 */
extern "C" void label_plugin_callback(const char *label_name, const char *symbol_type, unsigned long address) {
    if (label_name) {
        label_count++;
        std::cout << "###   [LABEL] " << label_name << " (Type: " << symbol_type << ", Address: 0x" << std::hex << address << std::dec << ")###\n\n\n";
    }
}

/**
 * Print the final statistics report to the console.
 */
extern "C" void plugin_print_stats(void) {
    // Close file and add final newline if open
    if (output_file != NULL) {
        fprintf(output_file, "\n");
        fclose(output_file);
        output_file = NULL;
    }
    std::cerr << "\n[INFO] Instructions written to: disasm_output.txt\n\n";
  
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║               INSTRUCTION STATISTICS                   ║\n";
    std::cout << "║ Architecture: " << g_current_arch;
    // Padding to keep the box format
    int padding = 41 - g_current_arch.length();
    for (int i = 0; i < padding; ++i) std::cout << " ";
    std::cout << "║\n";
    std::cout << "╠════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Total files processed:                 " << std::dec << file_count << " \n";
    std::cout << "║ Total sections processed:              " << std::dec << section_count << " \n";
    std::cout << "║ Total labels processed:                " << std::dec << label_count << " \n";
    std::cout << "║ Total instructions:                    " << std::dec << insn_count << " \n";
    std::cout << "║ Invalid instructions (BAD):            " << std::dec << bad_count << " \n";  

    #if defined(TARGET_I386) || defined(TARGET_x86_64)
    std::cout << "╠════════════════════════════════════════════════════════╣\n"; 
    std::cout << "║ Calls (CALL):                          " << std::dec << call_count << " \n";
    std::cout << "║ Jumps (JUMP):                          " << std::dec << jump_count << " \n";
    std::cout << "║ Returns (RET):                         " << std::dec << return_count << " \n";
    std::cout << "║ Instructions with REX.W:               " << std::dec << rex_count << " \n";
    std::cout << "║ Atomic operations (LOCK):              " << std::dec << lock_count << " \n";
    std::cout << "║ String operations (REP):               " << std::dec << rep_count << " \n";
    std::cout << "║ Stack canaries detected:               " << std::dec << canary_count << " \n";
    #endif

    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    std::cout << "║ Regex pattern dereference (file):      " << pattern_matches << "        \n";
    std::cout << "║ Regex pattern indirect call (file):    " << pattern_matches2 << "        \n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
}

/**
* Write the instruction to the output file in the format: address::mnemonic,operands|
*/
void write_instruction_to_file(const struct instr_info *ins, const std::string& operandString) {
   // Use C FILE I/O for speed (faster than C++ streams)
   fprintf(output_file, "%lx::%s,", ins->start_pc, ins->mnemonic);
  
   if (!operandString.empty()) {
       fprintf(output_file, "%s,", operandString.c_str());
   } else {
       fprintf(output_file, ",");
   }
   fprintf(output_file, "|");
}

/**
 * Count the type of instruction (call, jump, return) and increment the corresponding counter.
 */
void count_instruction_types(const struct instr_info *ins) {
    #if defined(TARGET_I386) || defined(TARGET_x86_64) // Only for x86 architectures
    if (ins->mnemonic[0] != '\0') {
        std::string m(ins->mnemonic);
        if (m.find("call") != std::string::npos) {
            call_count++;
        }
        else if (m[0] == 'j') {
            jump_count++;
        }
        else if (m.find("ret") != std::string::npos) {
            return_count++;
        }
    }
    #endif
}


void print_instruction_console(const instr_info *ins) {
    std::cout << "[Instruction #" << std::dec << insn_count << "] ";
    if (!nmemonicString.empty()) {
        std::cout << nmemonicString << " ";
        if (!operandString.empty()) {
            print_operands(operandString);
        }
    }
}

void print_operands(std::string operands) {
    if (operands.empty())
        return;

    int count = 1;
    for (char c : operands)
        if (c == ',') ++count;

    std::cout << " [OPERAND x" << count << "]";

    size_t start = 0;
    size_t len = operands.size();
    for (size_t i = 0; i <= len; ++i) {
        if (i == len || operands[i] == ',') {
            size_t token_start = start;
            size_t token_len = i - start;
            // trim leading spaces
            while (token_len && operands[token_start] == ' ') {
                ++token_start;
                --token_len;
            }
            std::cout << " " << operands.substr(token_start, token_len);
            start = i + 1;
        }
    }
}


/**
 * Analyze and print instruction prefixes, incrementing counters for atomic, string, canary, and REX.W instructions.
 */
void analyze_prefixes(const struct instr_info *ins) {
#if defined(TARGET_I386)  || defined(TARGET_x86_64) // Only for x86 architectures
    if (ins->last_lock_prefix >= 0) {
        std::cout << " # ATOMIC";
        lock_count++;
    }
    if (ins->last_repz_prefix >= 0 || ins->last_repnz_prefix >= 0) {
        std::cout << " # STRING-OP";
        rep_count++;
    }
    if (ins->active_seg_prefix != 0 && !operandString.empty()) {
        if (operandString.find(":0x28") != std::string::npos || operandString.find(":40") != std::string::npos) {
            std::cout << " # STACK-CANARY";
            canary_count++;
        }
    }
    if (ins->rex != 0 && (ins->rex & 0x8)) {
        std::cout << " # REX.W=1";
        rex_count++;
    }
    std::cout << "\n";
#else
    // for other architectures,  do nothing
    std::cout << "\n";
#endif
}



// Analiza regex desde el desensamblado (optimizado con pre-filtrado)
void detect_regex_patterns(std::string operands) {
    #if defined(TARGET_I386) || defined(TARGET_x86_64)
    if (operands.empty() || !pattern1 || !pattern2)
        return;
    
    const bool has_star  = operands.find('*') != std::string::npos;
    const bool has_paren = operands.find('(') != std::string::npos ||
                           operands.find('[') != std::string::npos;

    // pattern1: dereference / memory
    if (has_star || has_paren) {
        if (RE2::PartialMatch(operands, *pattern1))
            ++pattern_matches;
    }

    // pattern2: indirect call (*%reg)
    if (has_star) {
        if (RE2::PartialMatch(operands, *pattern2))
            ++pattern_matches2;
    }
    #endif
}
