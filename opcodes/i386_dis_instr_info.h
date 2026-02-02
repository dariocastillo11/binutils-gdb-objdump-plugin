/**
 * instr_info structure definition for i386 disassembler plugins
 */
#ifndef I386_DIS_INFO_H
#define I386_DIS_INFO_H
#include <stdint.h>
#include <stdbool.h>

#define MAX_OPERANDS 5
#define MAX_OPERAND_BUFFER_SIZE 128
#define MAX_CODE_LENGTH 15

/* Enums usados por struct instr_info */
#ifndef ADDRESS_MODE_DEFINED
enum address_mode { mode_16bit, mode_32bit, mode_64bit };
#define ADDRESS_MODE_DEFINED
#endif

#ifndef X86_64_ISA_DEFINED
enum x86_64_isa { amd64 = 1, intel64 };
#define X86_64_ISA_DEFINED
#endif

#ifndef EVEX_TYPE_DEFINED
enum evex_type { evex_default = 0, evex_from_legacy, evex_from_vex };
#define EVEX_TYPE_DEFINED
#endif

/* Struct instr_info for exchange between disassembler and plugins */
struct instr_info
{
  enum address_mode address_mode; /* Addressing mode: 16, 32, or 64 bit */

  /* Flags for the prefixes for the current instruction.  */
  int prefixes;

  /* REX prefix the current instruction.  */
  uint8_t rex;
  /* Bits of REX we've already used.  */
  uint8_t rex_used;

  /* Record W R4 X4 B4 bits for rex2.  */
  unsigned char rex2;
  /* Bits of rex2 we've already used.  */
  unsigned char rex2_used;
  unsigned char rex2_payload;

  bool need_modrm; 
  unsigned char condition_code;
  unsigned char need_vex;
  bool has_sib;

  /* Flags for ins->prefixes which we somehow handled when printing the
     current instruction.  */
  int used_prefixes;

  /* Flags for EVEX bits which we somehow handled when printing the
     current instruction.  */
  int evex_used;

  char obuf[MAX_OPERAND_BUFFER_SIZE]; 
  char *obufp;
  char *mnemonicendp;  
  const uint8_t *start_codep;
  uint8_t *codep;
  const uint8_t *end_codep;
  unsigned char nr_prefixes;
  signed char last_lock_prefix;
  signed char last_repz_prefix;
  signed char last_repnz_prefix;
  signed char last_data_prefix;
  signed char last_addr_prefix;
  signed char last_rex_prefix;
  signed char last_rex2_prefix;
  signed char last_seg_prefix;
  signed char fwait_prefix;
  /* The active segment register prefix.  */
  unsigned char active_seg_prefix;

  /* We can up to 14 ins->prefixes since the maximum instruction length is
     15bytes.  */
  uint8_t all_prefixes[MAX_CODE_LENGTH - 1];
  void *info;  /* disassemble_info * */

  struct { int mod, reg, rm; } modrm;
  struct { int scale, index, base; } sib;

  struct {
    int register_specifier;
    int length;
    int prefix;
    int mask_register_specifier;
    int scc;
    int ll;
    bool w;
    bool evex;
    bool v;
    bool zeroing;
    bool b;
    bool no_broadcast;
    bool nf;
  } vex;

  enum evex_type evex_type;

  /* Remember if the current op is a jump instruction.  */
  bool op_is_jump;

  bool two_source_ops;

  /* Record whether EVEX masking is used incorrectly.  */
  bool illegal_masking;

  /* Record whether the modrm byte has been skipped.  */
  bool has_skipped_modrm;

  unsigned char op_ad;
  signed char op_index[MAX_OPERANDS];
  bool op_riprel[MAX_OPERANDS];
  char *op_out[MAX_OPERANDS];
  uint64_t op_address[MAX_OPERANDS];
  uint64_t start_pc;
  
  /* On the 386's of 1988, the maximum length of an instruction is 15 bytes.
   *   (see topic "Redundant ins->prefixes" in the "Differences from 8086"
   *   section of the "Virtual 8086 Mode" chapter.)
   * 'pc' should be the address of this instruction, it will
   *   be used to print the target address if this is a relative jump or call
   * The function returns the length of this instruction in bytes.
   */

  char intel_syntax;
  bool intel_mnemonic;
  char open_char;
  char close_char;
  char separator_char;
  char scale_char;

  enum x86_64_isa isa64;
  
  void *priv;  /* dis_private * */
  
  /* Mnemonic and operand of the current instruction */
char mnemonic[64];
char operands[256];
};

#endif // I386_DIS_INFO_H
