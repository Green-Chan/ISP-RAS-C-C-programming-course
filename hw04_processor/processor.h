#ifndef __PROCESSOR_HDR
#define __PROCESSOR_HDR

//! Specific assembler language commands (the values, that represent commands in specific binary code)
enum {
    HALT = 0,

    IN_CMD,
    OUT_CMD,

    ADD,
    SUB,
    MUL,
    DIV,
    SQRT,

    POP,
    POP_REG,
    PUSH_VAL,
    PUSH_REG,

    JMP,
    JIF,
    CALL,
    RET,

    LESS,
    GREATER,
    OR,
    AND,
    NOT
};

//! Specific assembler language registers (the values, that represent registers in specific binary code)
enum {
    RAX = 0,
    RBX,
    RCX,
    RDX
};

//! Possible return values of process function.
enum {
    PROC_SUCCESS = 0,
    FILE_PROC_ERR,
    HEADER_PROC_ERR,
    OUT_OF_MEMORY_PROC_ERR,
    POP_FROM_EMPTY_STACK_PROC_ERR,
    NOT_ENOUGH_ARGS_ON_STACK_PROC_ERR,
    UNKNOWN_REGISTER_PROC_ERR,
    UNKNOWN_COMMAND_PROC_ERR,
    NO_HALT_PROC_ERR,
    BAD_JMP_ADDRESS_PROC_ERR,
    RET_WITHOUT_CALL_PROC_ERR
};

static const char header1[8] = {0, 'G', 'C', '1', '0', '0', '0', '0' };
static const char header2[8] = {0, 'G', 'C', '2', '0', '0', '0', '0' };
static const char header3[8] = {0, 'G', 'C', '3', '0', '0', '0', '0' };

extern int header_error;

//! Executes program on specific binary code.
//! @param [in]  file_path   Path to the file with binary code
//! @return 0 on success, error code otherwise
int process(const char *file_path);

//char *get_processor_file_error();

#endif
