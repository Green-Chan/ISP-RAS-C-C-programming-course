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
CLOSE_IN_FILE; \
fclose(file_out); \
return

int write_header(FILE *file_out) {
    if (fwrite(header, sizeof(header), 1, file_out) != 1) {
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

    FILE *file_out = fopen(file_out_path, "wb");
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
    while (cur_symb < file_in_data + file_in_size) {
        line += skip_spaces(&cur_symb, file_in_data + file_in_size);
        if (cur_symb + sizeof(HALT_STR) > file_in_data + file_in_size) {
            break; // No enough space for halt, file ends without halt
        }
        // Note: all commands are sizeof(HALT_STR) or less symbols length
        char buf[1 + sizeof(double)];
        size_t size = 0;
        if (strncmp(cur_symb, HALT_STR, sizeof(HALT_STR) - 1) == 0) {
            cur_symb += sizeof(HALT_STR) - 1;
            if (cur_symb == file_in_data + file_in_size || !isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = HALT;
            size = 1;
        } else if (strncmp(cur_symb, IN_STR, sizeof(IN_STR) - 1) == 0) {
            cur_symb += sizeof(IN_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = IN_CMD;
            size = 1;
        } else if (strncmp(cur_symb, OUT_STR, sizeof(OUT_STR) - 1) == 0) {
            cur_symb += sizeof(OUT_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = OUT_CMD;
            size = 1;
        } else if (strncmp(cur_symb, ADD_STR, sizeof(ADD_STR) - 1) == 0) {
            cur_symb += sizeof(ADD_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = ADD;
            size = 1;
        } else if (strncmp(cur_symb, SUB_STR, sizeof(SUB_STR) - 1) == 0) {
            cur_symb += sizeof(SUB_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = SUB;
            size = 1;
        } else if (strncmp(cur_symb, MUL_STR, sizeof(MUL_STR) - 1) == 0) {
            cur_symb += sizeof(MUL_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = MUL;
            size = 1;
        } else if (strncmp(cur_symb, DIV_STR, sizeof(DIV_STR) - 1) == 0) {
            cur_symb += sizeof(DIV_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = DIV;
            size = 1;
        } else if (strncmp(cur_symb, SQRT_STR, sizeof(SQRT_STR) - 1) == 0) {
            cur_symb += sizeof(SQRT_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            buf[0] = SQRT;
            size = 1;
        } else if (strncmp(cur_symb, POP_STR, sizeof(POP_STR) - 1) == 0) {
            cur_symb += sizeof(POP_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            line += skip_spaces(&cur_symb, file_in_data + file_in_size);
            if (cur_symb + sizeof(HALT_STR) - 1> file_in_data + file_in_size) {
                break; // No enough space for halt, file ends without halt
                // Note: all commands are less then sizeof(HALT_STR) symbols length
            } else if (strncmp(cur_symb, RAX_STR, sizeof(RAX_STR) - 1) == 0) {
                cur_symb += sizeof(RAX_STR) - 1;
                buf[0] = POP_REG;
                buf[1] = RAX;
                size = 2;
            } else if (strncmp(cur_symb, RBX_STR, sizeof(RBX_STR) - 1) == 0) {
                cur_symb += sizeof(RBX_STR) - 1;
                buf[0] = POP_REG;
                buf[1] = RBX;
                size = 2;
            } else if (strncmp(cur_symb, RCX_STR, sizeof(RCX_STR) - 1) == 0) {
                cur_symb += sizeof(RCX_STR) - 1;
                buf[0] = POP_REG;
                buf[1] = RCX;
                size = 2;
            } else if (strncmp(cur_symb, RDX_STR, sizeof(RDX_STR) - 1) == 0) {
                cur_symb += sizeof(RDX_STR) - 1;
                buf[0] = POP_REG;
                buf[1] = RDX;
                size = 2;
            } else {
                // that is pop without arguments
                buf[0] = POP;
                size = 1;
            }
        } else if (strncmp(cur_symb, PUSH_STR, sizeof(PUSH_STR) - 1) == 0) {
            cur_symb += sizeof(PUSH_STR) - 1;
            if (!isspace(*cur_symb)) {
                if (err_line) { *err_line = line; }
                END_ASM_AND_RETURN UNKNOWN_CMD_ASM_ERR;
            }
            line += skip_spaces(&cur_symb, file_in_data + file_in_size);
            if (cur_symb + sizeof(HALT_STR) - 1 > file_in_data + file_in_size) {
                break; // No enough space for halt, file ends without halt
                // Note: all commands are less then sizeof(HALT_STR) symbols length
            } else if (strncmp(cur_symb, RAX_STR, sizeof(RAX_STR) - 1) == 0) {
                cur_symb += sizeof(RAX_STR) - 1;
                buf[0] = PUSH_REG;
                buf[1] = RAX;
                size = 2;
            } else if (strncmp(cur_symb, RBX_STR, sizeof(RBX_STR) - 1) == 0) {
                cur_symb += sizeof(RBX_STR) - 1;
                buf[0] = PUSH_REG;
                buf[1] = RBX;
                size = 2;
            } else if (strncmp(cur_symb, RCX_STR, sizeof(RCX_STR) - 1) == 0) {
                cur_symb += sizeof(RCX_STR) - 1;
                buf[0] = PUSH_REG;
                buf[1] = RCX;
                size = 2;
            } else if (strncmp(cur_symb, RDX_STR, sizeof(RDX_STR) - 1) == 0) {
                cur_symb += sizeof(RDX_STR) - 1;
                buf[0] = PUSH_REG;
                buf[1] = RDX;
                size = 2;
            } else {
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
                char *end_ptr = NULL;
                double darg = strtod(arg, &end_ptr);
                if (end_ptr != arg + len) {
                if (err_line) { *err_line = line; }
                    END_ASM_AND_RETURN UNKNOWN_ARG_ASM_ERR;
                }
                buf[0] = PUSH_VAL;
                memcpy(buf + 1, &darg, sizeof(double));
                size = 1 + sizeof(double);
                cur_symb += len;
            }
        } else {
            END_ASM_AND_RETURN UNKNOWN_ARG_ASM_ERR;
        }
        assert(size != 0);
        if (fwrite(buf, size, 1, file_out) != 1) {
            END_ASM_AND_RETURN FILE_OUT_ASM_ERR;
        }
        if (buf[0] == 0) { // That was HALT
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
        }
    }
    // We are here if there was no halt
    END_ASM_AND_RETURN NO_HALT_ASM_ERR;
}


