#include "processor.h"
#include "assembler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <inttypes.h>
#include <assert.h>

char *get_disassembler_file_in_error()
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

#define CLOSE_IN_FILE \
UnmapViewOfFile((LPCVOID)file_in_data); \
CloseHandle(file_in_mapping); \
CloseHandle(file_in_handle)

#define END_DIS_AND_RETURN \
CLOSE_IN_FILE; \
fclose(file_out); \
return

int write_register(char *buf, size_t *size, char reg) {
    // We know that all registers are rax (code 0), rbx (1), rcx (2), rdx (3)
    if ((unsigned char) reg > 3) {
        // Not a register
        return 1;
    } else {
        buf[*size + 0] = 'r';
        buf[*size + 1] = 'a' + reg;
        buf[*size + 2] = 'x';
        *size += 3;
        return 0;
    }
}

int check_header_disasm(const char **cur_char, size_t file_size) {
    if (file_size < sizeof(header1)) {
        return 1;
    }
    if (memcmp(*cur_char, header1, sizeof(header1)) == 0 ||
        memcmp(*cur_char, header2, sizeof(header2)) == 0 ) {
        // Both versions could be disassembled and headers have the same size
        (*cur_char) += sizeof(header1);
         return 0;
    }
    return 1;
}

int disassemble(const char *file_in_path, const char *file_out_path) {
    HANDLE file_in_handle = CreateFile(file_in_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_in_handle == INVALID_HANDLE_VALUE) {
        return FILE_IN_DISASM_ERR;
    }
    uint32_t file_in_size = GetFileSize(file_in_handle, NULL);
    if (file_in_size == INVALID_FILE_SIZE) {
        return FILE_IN_DISASM_ERR;
    }
    HANDLE file_in_mapping = CreateFileMapping(file_in_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (file_in_mapping == NULL) {
        return FILE_IN_DISASM_ERR;
    }
    const char *file_in_data = (const char *)MapViewOfFile(file_in_mapping, FILE_MAP_READ, 0, 0, 0);
    if (file_in_data == NULL) {
        return FILE_IN_DISASM_ERR;
    }

    const char *cur_cmd = file_in_data;
    if (check_header_disasm(&cur_cmd, file_in_size)) {
        CLOSE_IN_FILE;
        return HEADER_DISASM_ERR;
    }

    FILE *file_out = fopen(file_out_path, "wb");
    if (file_out == NULL) {
        CLOSE_IN_FILE;
        return FILE_OUT_DISASM_ERR;
    }

    while (cur_cmd < file_in_data + file_in_size) {
        char buf[20];
        size_t size = 0;
        switch (*cur_cmd) {
        case HALT:
            if (fwrite(HALT_STR, sizeof(HALT_STR) - 1, 1, file_out) != 1) {
                END_DIS_AND_RETURN FILE_OUT_DISASM_ERR;
            }
            if (fclose(file_out) != 0) {
                CLOSE_IN_FILE;
                END_DIS_AND_RETURN FILE_OUT_DISASM_ERR;
            }
            if (UnmapViewOfFile((LPCVOID)file_in_data) == 0) {
                return FILE_IN_DISASM_ERR;
            }
            if (CloseHandle(file_in_mapping) == 0) {
                return FILE_IN_DISASM_ERR;
            }
            if (CloseHandle(file_in_handle) == 0) {
                return FILE_IN_DISASM_ERR;
            }
            return DISASM_SUCCESS;
        case IN_CMD:
            memcpy(buf, IN_STR, sizeof(IN_STR) - 1);
            size = sizeof(IN_STR) - 1;
            cur_cmd++;
            break;
        case OUT_CMD:
            memcpy(buf, OUT_STR, sizeof(OUT_STR) - 1);
            size = sizeof(OUT_STR) - 1;
            cur_cmd++;
            break;
        case ADD:
            memcpy(buf, ADD_STR, sizeof(ADD_STR) -1 );
            size = sizeof(ADD_STR) - 1;
            cur_cmd++;
            break;
        case SUB:
            memcpy(buf, SUB_STR, sizeof(SUB_STR) -1 );
            size = sizeof(SUB_STR) - 1;
            cur_cmd++;
            break;
        case MUL:
            memcpy(buf, MUL_STR, sizeof(MUL_STR) -1 );
            size = sizeof(MUL_STR) - 1;
            cur_cmd++;
            break;
        case DIV:
            memcpy(buf, DIV_STR, sizeof(DIV_STR) - 1);
            size = sizeof(DIV_STR) - 1;
            cur_cmd++;
            break;
        case SQRT:
            memcpy(buf, SQRT_STR, sizeof(SQRT_STR) - 1);
            size = sizeof(SQRT_STR) - 1;
            cur_cmd++;
            break;
        case POP:
            memcpy(buf, POP_STR, sizeof(POP_STR) - 1);
            size = sizeof(POP_STR) - 1;
            cur_cmd++;
            break;
        case POP_REG:
            memcpy(buf, POP_STR, sizeof(POP_STR) - 1);
            size = sizeof(POP_STR) - 1;
            cur_cmd++;
            buf[size] = ' ';
            size++;
            if (write_register(buf, &size, *cur_cmd)) {
                END_DIS_AND_RETURN UNKNOWN_REG_DISASM_ERR;
            }
            cur_cmd++;
            break;
        case PUSH_VAL:
            memcpy(buf, PUSH_STR, sizeof(PUSH_STR) - 1);
            size = sizeof(PUSH_STR) - 1;
            cur_cmd++;
            buf[size] = ' ';
            size++;
            if (cur_cmd + sizeof(double) > file_in_data + file_in_size) {
                END_DIS_AND_RETURN NO_HALT_PROC_ERR;
            }
            // -2 to keep space for "\r\n" in the end
            int n = snprintf(buf + size, sizeof(buf) - size - 2, "%lf", *((double *)cur_cmd));
            size += (n < sizeof(buf) - size - 2 ? n : sizeof(buf) - size - 2);
            cur_cmd += sizeof(double);
            break;
        case PUSH_REG:
            memcpy(buf, PUSH_STR, sizeof(PUSH_STR) - 1);
            size = sizeof(PUSH_STR) - 1;
            cur_cmd++;
            buf[size] = ' ';
            size++;
            if (write_register(buf, &size, *cur_cmd)) {
                END_DIS_AND_RETURN UNKNOWN_REG_DISASM_ERR;
            }
            cur_cmd++;
            break;
        case JMP:
            {
                memcpy(buf, JMP_STR, sizeof(JMP_STR) - 1);
                size = sizeof(JMP_STR) - 1;
                cur_cmd++;
                buf[size++] = ' ';
                if (cur_cmd + sizeof(size_t) > file_in_data + file_in_size) {
                    END_DIS_AND_RETURN NO_HALT_PROC_ERR;
                }
                // -2 to keep space for "\r\n" in the end
                size_t abs_addr = cur_cmd - file_in_data + sizeof(size_t) + *((size_t *)cur_cmd);
                int n = snprintf(buf + size, sizeof(buf) - size - 2, "%Id", abs_addr);
                size += (n < sizeof(buf) - size - 2 ? n : sizeof(buf) - size - 2);
                cur_cmd += sizeof(size_t);
                break;
            }
        default: END_DIS_AND_RETURN UNKNOWN_CMD_DISASM_ERR;
        }
        buf[size++] = '\r';
        buf[size++] = '\n';
        if (fwrite(buf, size, 1, file_out) != 1) {
            END_DIS_AND_RETURN FILE_OUT_DISASM_ERR;
        }
    }
    END_DIS_AND_RETURN NO_HALT_DISASM_ERR;
}
