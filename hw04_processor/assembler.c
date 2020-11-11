#include "processor.h"
#include "assembler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>
#include <assert.h>

char *get_assembler_file_in_error()
{
    LPSTR messageBuffer = NULL;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    char *error = calloc(size + 1, 1);
    memcpy(error, messageBuffer, size);
    error[size] = 0;
    LocalFree(messageBuffer);
    return error;
}

#define CLOSE_IN_FILE UnmapViewOfFile((LPCVOID)file_in_data); CloseHandle(file_in_mapping); CloseHandle(file_in_handle)

#define END_ASM_AND_RETURN \
destruct_stack(label, &label_tab); \
destruct_stack(size_t, &jmp_args); \
CLOSE_IN_FILE; \
fclose(file_out); \
return

int write_header(FILE *file_out) {
    if (fwrite(header3, sizeof(header3), 1, file_out) != 1) {
        return 1;
    }
    return 0;
}

size_t skip_spaces(const char **cur_symb, const char *end) {
    size_t lines = 0;
    while (*cur_symb < end && isspace(**cur_symb)) {
        if (**cur_symb == '\n') {
            lines++;
        }
        (*cur_symb)++;
    }
    return lines;
}


//! Compares the first word (the sequence of not space symbols to the end of file or first space symbol)
//! at @c cur_symb with command or register at @c cmd. If they are equal, sets @c cur_symb to the next
//! symbol after the first word.
//!
//! @note @c cmd should be static const char array from assembler.h so that @c sizeof(cmd) is the size of
//! @c cmd
//!
//! @param [in]      cmd        String representation of command or register
//! @param [in, out] cur_symb   Pointer to the pointer to the current symbol of text being parsing
//! @param [in]      file_end   Pointer to the symbol after last symbol of the text
//! @return 1 if the first word is equal to the specified command or register, 0 otherwise
#define check_cmd(cmd, cur_symb, file_end) _check_cmd(cmd, sizeof(cmd) - 1, cur_symb, file_end)

int _check_cmd(const char *cmd, size_t cmd_size, const char **cur_symb, const char *file_end) {
    if (strncmp(*cur_symb, cmd, cmd_size) == 0 && (*cur_symb + cmd_size == file_end || isspace(*(*cur_symb + cmd_size)))) {
        *cur_symb += cmd_size;
        return 1;
    } else {
        return 0;
    }
}

//! Compares the first word (the sequence of not space symbols to the end of file or first space symbol)
//! at @c cur_symb with string representations of existing registers. If the first word equal to some of
//! them, sets @c cur_symb to the next symbol after the word and returns the code of the corresponding
//! register.
//!
//! @param [in, out] cur_symb   Pointer to the pointer to the current symbol of text being parsing
//! @param [in]      file_end   Pointer to the symbol after last symbol of the text
//! @return If the first word is equal to some register string representation, code of the register. -1 otherwise
char get_register(const char **cur_symb, const char *file_end) {
    // Register can be checked the same way as commands
    if (check_cmd(RAX_STR, cur_symb, file_end)) {
        return RAX;
    } else if (check_cmd(RBX_STR, cur_symb, file_end)) {
        return RBX;
    } else if (check_cmd(RCX_STR, cur_symb, file_end)) {
        return RCX;
    } else if (check_cmd(RDX_STR, cur_symb, file_end)) {
        return RDX;
    } else {
        // not a register
        return -1;
    }
}

typedef struct label_s {
    const char *name;
    size_t name_len;
    size_t abs_addr;
    int line_no;
} label;

#if DEBUG > 2
    #error DEBUG-levels more than 2 are not allowed
#endif

// Container for label_tab
#define STACK_TYPE label
#include "stack.h"
#undef STACK_TYPE

// Container for jmp_args
#define STACK_TYPE size_t
#include "stack.h"
#undef STACK_TYPE

//! Finds the first entry of label in label table
//!
//! @param [in]  thou        Pointer to the label table
//! @param [in]  label_name  Pointer to the begin of the label name
//! @param [in]  label_len   Length of the label name
//! @return The index of the first found entry. Size of label_tab if there is no entry for this label.
size_t find_label(TEMPLATE(label, stack) *thou, const char *label_name, size_t label_len) {
    size_t i = 0;
    for (; i < thou->size; i++) {
        if (strncmp(thou->data[i].name, label_name, label_len) == 0) {
            break;
        }
    }
    return i;
}


int get_label(const char **cur_symb, const char *file_end, TEMPLATE(label, stack) *label_tab,
              TEMPLATE(size_t, stack) *jmp_args, size_t *line, char *label_idx_place, size_t cur_byte) {
    *line += skip_spaces(cur_symb, file_end);
    size_t len = 0;
    while (*cur_symb + len < file_end && (isalpha((*cur_symb)[len]) || isdigit((*cur_symb)[len]))) {
        len++;
    }
    if (*cur_symb + len >= file_end) {
        // Where is halt command?
        return NO_HALT_ASM_ERR;
    }
    if (isspace((*cur_symb)[len])) {
        // That is correct label [cur_symb; cur_symb + len)
        size_t label_idx = find_label(label_tab, *cur_symb, len);
        if (label_idx == label_tab->size) {
            // That is a new label, and we don't know it's address yet
            label new_label = {*cur_symb, len, (size_t) -1ll, *line};
            if (push_stack(label, label_tab, new_label)) {
                return OUT_OF_MEMORY_ASM_ERR;
            }
        }
        // Put an index in the specified place
        assert (label_idx == find_label(label_tab, *cur_symb, len));
        memcpy(label_idx_place, &label_idx, sizeof(size_t));
        // And memorize address of the argument (current address plus one)
        if (push_stack(size_t, jmp_args, cur_byte + 1)) {
            return OUT_OF_MEMORY_ASM_ERR;
        }
        *cur_symb += len;
        return 0;
    } else {
        // That is not a label
        return UNKNOWN_ARG_ASM_ERR;
    }
}


int assemble(const char *file_in_path, const char *file_out_path, int *err_line) {
    HANDLE file_in_handle = CreateFile(file_in_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_in_handle == INVALID_HANDLE_VALUE) {
        return FILE_IN_ASM_ERR;
    }
    uint32_t file_in_size = GetFileSize(file_in_handle, NULL);
    if (file_in_size == INVALID_FILE_SIZE) {
        return FILE_IN_ASM_ERR;
    }
    HANDLE file_in_mapping = CreateFileMapping(file_in_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (file_in_mapping == NULL) {
        return FILE_IN_ASM_ERR;
    }
    const char *file_in_data = (const char *)MapViewOfFile(file_in_mapping, FILE_MAP_READ, 0, 0, 0);
    if (file_in_data == NULL) {
        return FILE_IN_ASM_ERR;
    }

    FILE *file_out = fopen(file_out_path, "w+b");
    if (file_out == NULL) {
        CLOSE_IN_FILE;
        return FILE_OUT_ASM_ERR;
    }

    if (write_header(file_out)) {
        CLOSE_IN_FILE;
        return FILE_OUT_ASM_ERR;
    }

    const char *cur_symb = file_in_data;
    size_t line = 1;
    size_t cur_byte = sizeof(header2);
    TEMPLATE(label, stack) label_tab;
    construct_stack(label, &label_tab);
    TEMPLATE(size_t, stack) jmp_args;
    construct_stack(size_t, &jmp_args);

    while (cur_symb < file_in_data + file_in_size) {
        line += skip_spaces(&cur_symb, file_in_data + file_in_size);
        if (cur_symb + sizeof(HALT_STR) - 1 > file_in_data + file_in_size) {
            break; // No enough space for halt, file ends without halt
        }
        // Note: all commands are sizeof(HALT_STR) - 1 or less symbols length
        char buf[1 + sizeof(double)];
        size_t size = 0;
        if (check_cmd(HALT_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = HALT;
            // Write command into file
            if (fwrite(buf, 1, 1, file_out) != 1) {
                END_ASM_AND_RETURN FILE_OUT_ASM_ERR;
            }

            // Fill in jmp arguments
            for (size_t i = 0; i < jmp_args.size; i++) {
                if (fseek(file_out, jmp_args.data[i], SEEK_SET)) {
                    END_ASM_AND_RETURN FILE_OUT_ASM_ERR;
                }
                size_t label_idx;
                if (fread(&label_idx, sizeof(size_t), 1, file_out) != 1) {
                    END_ASM_AND_RETURN FILE_OUT_ASM_ERR;
                }
                assert(label_idx < label_tab.size);
                if (label_tab.data[label_idx].abs_addr == (size_t) -1ll) {
                    *err_line = label_tab.data[label_idx].line_no;
                    END_ASM_AND_RETURN UNDEFINED_LABEL_ASM_ERR;
                }
                if (fseek(file_out, jmp_args.data[i], SEEK_SET)) {
                    END_ASM_AND_RETURN FILE_OUT_ASM_ERR;
                }
                size_t rel_addr = label_tab.data[label_idx].abs_addr - (jmp_args.data[i] + sizeof(size_t));
                if (fwrite(&rel_addr, sizeof(size_t), 1, file_out) != 1) {
                    END_ASM_AND_RETURN FILE_OUT_ASM_ERR;
                }
            }

            // Destruct label_tab and jmp_args
            destruct_stack(label, &label_tab);
            destruct_stack(size_t, &jmp_args);
            // Close files
            if (fclose(file_out) != 0) {
                CLOSE_IN_FILE;
                return FILE_OUT_ASM_ERR;
            }
            if (UnmapViewOfFile((LPCVOID)file_in_data) == 0) {
                return FILE_IN_ASM_ERR;
            }
            if (CloseHandle(file_in_mapping) == 0) {
                return FILE_IN_ASM_ERR;
            }
            if (CloseHandle(file_in_handle) == 0) {
                return FILE_IN_ASM_ERR;
            }
            return ASM_SUCCESS;
        } else if (check_cmd(IN_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = IN_CMD;
            size = 1;
        } else if (check_cmd(OUT_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = OUT_CMD;
            size = 1;
        } else if (check_cmd(ADD_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = ADD;
            size = 1;
        } else if (check_cmd(SUB_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = SUB;
            size = 1;
        } else if (check_cmd(MUL_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = MUL;
            size = 1;
        } else if (check_cmd(DIV_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = DIV;
            size = 1;
        } else if (check_cmd(SQRT_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = SQRT;
            size = 1;
        } else if (check_cmd(POP_STR, &cur_symb, file_in_data + file_in_size)) {
            line += skip_spaces(&cur_symb, file_in_data + file_in_size);
            if (cur_symb + sizeof(HALT_STR) - 1 > file_in_data + file_in_size) {
                break; // No enough space for halt, file ends without halt
                // Note: all commands are less then sizeof(HALT_STR) - 1 symbols length
            }
            char reg = get_register(&cur_symb, file_in_data + file_in_size);
            if (reg == (char) -1) {
                // that is pop without arguments
                buf[0] = POP;
                size = 1;
            } else {
                buf[0] = POP_REG;
                buf[1] = reg;
                size = 2;
            }
        } else if (check_cmd(PUSH_STR, &cur_symb, file_in_data + file_in_size)) {
            line += skip_spaces(&cur_symb, file_in_data + file_in_size);
            if (cur_symb + sizeof(HALT_STR) - 1 > file_in_data + file_in_size) {
                break; // No enough space for halt, file ends without halt
                // Note: all commands are less then sizeof(HALT_STR) - 1 symbols length
            }
            char reg = get_register(&cur_symb, file_in_data + file_in_size);
            if (reg == (char) -1) {
                // Form a null-terminated string to use strtod
                size_t len = 0;
                for (const char *p = cur_symb; p < file_in_data + file_in_size; p++) {
                    if (isspace(*p)) {
                        break;
                    }
                    len++;
                }
                assert(len > 0);
                char arg[len + 1];
                memcpy(arg, cur_symb, len);
                arg[len] = 0;
                // Get double argument from string
                char *end_ptr = NULL;
                double darg = strtod(arg, &end_ptr);
                if (end_ptr != arg + len) {
                    // That is not a double (and not a register)
                    if (err_line) { *err_line = line; }
                    END_ASM_AND_RETURN UNKNOWN_ARG_ASM_ERR;
                }
                buf[0] = PUSH_VAL;
                memcpy(buf + 1, &darg, sizeof(double));
                size = 1 + sizeof(double);
                cur_symb += len;
            } else {
                buf[0] = PUSH_REG;
                buf[1] = reg;
                size = 2;
            }
        } else if (check_cmd(JMP_STR, &cur_symb, file_in_data + file_in_size)) {
            int label_err = get_label(&cur_symb, file_in_data + file_in_size, &label_tab, &jmp_args, &line, buf + 1, cur_byte);
            if (label_err) {
                END_ASM_AND_RETURN label_err;
            } else {
                buf[0] = JMP;
                // get_label wrote size_t argument at address (buf + 1)
                size = 1 + sizeof(size_t);
            }
        } else if (check_cmd(JIF_STR, &cur_symb, file_in_data + file_in_size)) {
            int label_err = get_label(&cur_symb, file_in_data + file_in_size, &label_tab, &jmp_args, &line, buf + 1, cur_byte);
            if (label_err) {
                END_ASM_AND_RETURN label_err;
            } else {
                buf[0] = JIF;
                // get_label wrote size_t argument at address (buf + 1)
                size = 1 + sizeof(size_t);
            }
        } else if (check_cmd(CALL_STR, &cur_symb, file_in_data + file_in_size)) {
            int label_err = get_label(&cur_symb, file_in_data + file_in_size, &label_tab, &jmp_args, &line, buf + 1, cur_byte);
            if (label_err) {
                END_ASM_AND_RETURN label_err;
            } else {
                buf[0] = CALL;
                // get_label wrote size_t argument at address (buf + 1)
                size = 1 + sizeof(size_t);
            }
        } else if (check_cmd(RET_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = RET;
            size = 1;
        } else if (check_cmd(LESS_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = LESS;
            size = 1;
        } else if (check_cmd(GREATER_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = GREATER;
            size = 1;
        } else if (check_cmd(OR_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = OR;
            size = 1;
        } else if (check_cmd(AND_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = AND;
            size = 1;
        } else if (check_cmd(NOT_STR, &cur_symb, file_in_data + file_in_size)) {
            buf[0] = NOT;
            size = 1;
        } else {
            // That is not a command
            // Check if it is a label
            size_t len = 0;
            while (true) {
                if (cur_symb + len + sizeof(HALT_STR) - 1 >= file_in_data + file_in_size) {
                    // Where is halt command?
                    END_ASM_AND_RETURN NO_HALT_ASM_ERR;
                }
                if (isalpha(cur_symb[len]) || isdigit(cur_symb[len])) {
                    // That could be a label
                    len++;
                } else if (cur_symb[len] == ':' && isspace(cur_symb[len + 1])) {
                    // That is a label!
                    break;
                } else {
                    // That is not a label
                    // So what is it?
                    if (err_line) { *err_line = line; }
                    END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
                }
            }
            // That is a label [cur_symb; cur_symb + len)
            size_t label_idx = find_label(&label_tab, cur_symb, len);
            if (label_idx == label_tab.size) {
                // That is a new label
                label new_label = {cur_symb, len, cur_byte, line};
                if (push_stack(label, &label_tab, new_label)) {
                    END_ASM_AND_RETURN OUT_OF_MEMORY_ASM_ERR;
                }
            } else {
                // That is an old label, we should fill in its address.
                // If it is already filled in, this label is being defined second time
                if (label_tab.data[label_idx].abs_addr != (size_t) -1ll) {
                    if (err_line) { *err_line = line; }
                    END_ASM_AND_RETURN MULTIPLE_DEFINE_OF_LABEL_ASM_ERR;
                }
                // Notice, that this will be an error, if the DEBUG
                // level of stack more than 2, so we should not allow
                // such levels
                label_tab.data[label_idx].abs_addr = cur_byte;
            }
            // plus one is because of ':'
            cur_symb += len + 1;
            // If that was a label, we have nothing to write in file.
            continue;
        }
        // Write command into file
        assert(size != 0);
        if (fwrite(buf, size, 1, file_out) != 1) {
            END_ASM_AND_RETURN FILE_OUT_ASM_ERR;
        }
        cur_byte += size;
    }
    // We are here if there was no halt
    END_ASM_AND_RETURN NO_HALT_ASM_ERR;
}


