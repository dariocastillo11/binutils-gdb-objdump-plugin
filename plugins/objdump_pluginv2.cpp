#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <re2/re2.h>
#include "../opcodes/i386_dis_instr_info.h"
#include "objdump_plugin.h"
#if defined(TARGET_I386)
#include "objdump_plugin_i386.h"
#elif defined(TARGET_X86_64)
#include "objdump_plugin_x86_64.h"
#elif defined(TARGET_ARM)
#include "objdump_plugin_arm.h"
#endif
static unsigned long pattern_matches = 0;  // DEREFERENCE_PATTERN
static unsigned long pattern_matches2 = 0;  // INDIRECT_CALL_NO_OFFSET_PATTERN

static RE2* pattern1 = nullptr;
static RE2* pattern2 = nullptr;

// Statistical counters
static unsigned long insn_count = 0;

static std::string operandString;
static std::string nmemonicString;

// Output file
static FILE *output_file = NULL;

void write_instruction_to_file(const struct instr_info *ins, const std::string& operandString);
void detect_regex_patterns(const std::string& operands);

extern "C" void plugin_start(void) {
    operandString.reserve(40);
    nmemonicString.reserve(16);
    output_file = fopen("disasm_output.txt", "w");
    if (output_file == NULL) {
        std::cerr << "Error: Could not open output file\n";
        return;
    }
    try
    {
        pattern1 = new RE2(R"((\*)?("?[&]?[a-zA-Z0-9_-]+"?|0x[0-9a-fA-F]+|-0x[0-9a-fA-F]+)?[\(\[][^)\]]+[\)\]])");
        pattern2 = new RE2(R"(^\*(%[a-z0-9]+)$)");
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: Could not compile regex patterns " << '\n';
    }
    
}

// Hook llamado al final
extern "C" void plugin_cleanup(void) {
    if (output_file != NULL) {
        fprintf(output_file, "\n");
        fclose(output_file);
        output_file = NULL;
    }
    if(pattern1!= nullptr){
        delete pattern1;
        pattern1 = nullptr;
    }
    if(pattern2!= nullptr){
        delete pattern2;
        pattern2 = nullptr;
    }
}

extern "C" void instruction_callback(struct instr_info *ins) {
    if (!ins) {
        return;
    }
    insn_count++;
    operandString.assign(ins->operands);
    nmemonicString.assign(ins->mnemonic);
    detect_regex_patterns(operandString);
    write_instruction_to_file(ins, operandString);
}

void write_instruction_to_file(const struct instr_info *ins, const std::string& operandString) {
    if (output_file != NULL) {
        fprintf(output_file, "%lx::%s,", ins->start_pc, nmemonicString.c_str());
        if (!operandString.empty()) {
            fprintf(output_file, "%s,", operandString.c_str());
        } else {
            fprintf(output_file, ",");
        }
        fprintf(output_file, "|\n");
    }
}

void detect_regex_patterns(const std::string& operands) {
    if (operands.empty() || pattern1 == nullptr || pattern2 == nullptr) return;
    try {
        if (RE2::PartialMatch(operands, *pattern1)) pattern_matches++;
        if (RE2::PartialMatch(operands, *pattern2)) pattern_matches2++;
    } catch (const std::exception& e) {
        std::cerr << "Regex error: " << e.what() << '\n';
    }
}

extern "C" void plugin_print_stats(void) {
    if(output_file != NULL) {
        fprintf(output_file, "\n");
        fclose(output_file);
        output_file = NULL;
        fprintf(stdout, "[INFO] Instructions written to: disasm_output.txt\n");
    }
    std::string stats =
        "\n[INFO] Instructions written to: disasm_output.txt\n\n"
        "╔════════════════════════════════════════════════════════╗\n"
        "║               INSTRUCTION STATISTICS  v2                 ║\n"
        "╠════════════════════════════════════════════════════════╣\n";
    stats += "║ Total instructions:                    " + std::to_string(insn_count) + " \n";
    stats += "╚════════════════════════════════════════════════════════╝\n";
    stats += "║ Regex pattern dereference (file):      " + std::to_string(pattern_matches) + "        \n";
    stats += "║ Regex pattern indirect call (file):    " + std::to_string(pattern_matches2) + "        \n";
    stats += "╚════════════════════════════════════════════════════════╝\n\n";
    std::cout << stats;
    std::cerr << stats;

    
}