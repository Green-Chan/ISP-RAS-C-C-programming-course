#ifndef __PROCESSOR_ASSEMBLER
#define __PROCESSOR_ASSEMBLER

//! Possible return values of assemble function.
enum {
    ASM_SUCCESS = 0,
    FILE_IN_ASM_ERR,
    FILE_OUT_ASM_ERR,
    UNKNOWN_CMD_ASM_ERR,
    UNKNOWN_ARG_ASM_ERR,
    NO_HALT_ASM_ERR,
    OUT_OF_MEMORY_ASM_ERR,
    UNDEFINED_LABEL_ASM_ERR,
    MULTIPLE_DEFINE_OF_LABEL_ASM_ERR
};

//! Possible return values of disassemble function.
enum {
    DISASM_SUCCESS = 0,
    FILE_IN_DISASM_ERR,
    FILE_OUT_DISASM_ERR,
    HEADER_DISASM_ERR,
    UNKNOWN_CMD_DISASM_ERR,
    UNKNOWN_REG_DISASM_ERR,
    NO_HALT_DISASM_ERR
};

//! String representation of specific assembler language commands
static const char HALT_STR[] = "halt";
static const char IN_STR[] = "in";
static const char OUT_STR[] = "out";
static const char ADD_STR[] = "add";
static const char SUB_STR[] = "sub";
static const char MUL_STR[] = "mul";
static const char DIV_STR[] = "div";
static const char SQRT_STR[] = "sqrt";
static const char POP_STR[] = "pop";
static const char PUSH_STR[] = "push";
static const char JMP_STR[] = "jmp";

//! String representation of specific assembler language registers
static const char RAX_STR[] = "rax";
static const char RBX_STR[] = "rbx";
static const char RCX_STR[] = "rcx";
static const char RDX_STR[] = "rdx";


//! Assembles code on specific assembler language in specific binary code.
//! @param [in]  file_in_path   Path to the file with assembler language
//! @param [in]  file_out_path  Path to the file, where to write binary code
//! @param [out] err_line       Pointer to the variable, where the number of the line
//!                             in which the first error occurred will be stored
//!                             in case of unsuccessful parsing. (Line of file at file_in_path.)
//! @return 0 on success, error code otherwise
//!
//! @attention If @c file_out_path exists, it will be overwritten
int assemble(const char *file_in_path, const char *file_out_path, int *err_line);

// char *get_assembler_file_in_error();

//! Disassembles specific binary code in code on specific assembler language.
//! @param [in]  file_in_path   Path to the file with binary code
//! @param [in]  file_out_path  Path to the file, where to write code on assembler language
//! @return 0 on success, error code otherwise
//!
//! @attention If @c file_out_path exists, it will be overwritten
int disassemble(const char *file_in_path, const char *file_out_path);

//char *get_disassembler_file_in_error();

#endif
