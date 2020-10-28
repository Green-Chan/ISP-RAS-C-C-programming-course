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

int check_header_disasm(const char **cur_char, size_t file_size) {
    if (file_size < sizeof(header) || memcmp(*cur_char, header, sizeof(header)) != 0) {
        return 1;
    }
    (*cur_char) += sizeof(header);
    return 0;
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
            if (fwrite(HALT_STR, sizeof(HALT_STR), 1, file_out) != 1) {
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
            memcpy(buf, IN_STR, sizeof(IN_STR));
            size = sizeof(IN_STR);
            cur_cmd++;
            break;
        case OUT_CMD:
            memcpy(buf, OUT_STR, sizeof(OUT_STR));
            size = sizeof(OUT_STR);
            cur_cmd++;
            break;
        case ADD:
            memcpy(buf, ADD_STR, sizeof(ADD_STR));
            size = sizeof(ADD_STR);
            cur_cmd++;
            break;
        case SUB:
            memcpy(buf, SUB_STR, sizeof(SUB_STR));
            size = sizeof(SUB_STR);
            cur_cmd++;
            break;
        case MUL:
            memcpy(buf, MUL_STR, sizeof(MUL_STR));
            size = sizeof(MUL_STR);
            cur_cmd++;
            break;
        case DIV:
            memcpy(buf, DIV_STR, sizeof(DIV_STR));
            size = sizeof(DIV_STR);
            cur_cmd++;
            break;
        case SQRT:
            memcpy(buf, SQRT_STR, sizeof(SQRT_STR));
            size = sizeof(SQRT_STR);
            cur_cmd++;
            break;
        case POP:
            memcpy(buf, POP_STR, sizeof(POP_STR));
            size = sizeof(POP_STR);
            cur_cmd++;
            break;
        case POP_REG:
            memcpy(buf, POP_STR, sizeof(POP_STR));
            size = sizeof(POP_STR);
            cur_cmd++;
            buf[size] = ' ';
            size++;
            switch (*cur_cmd) {
                case RAX:
                    memcpy(buf + size, RAX_STR, sizeof(RAX_STR));
                    size += sizeof(RAX_STR);
                    break;
                case RBX:
                    memcpy(buf + size, RBX_STR, sizeof(RBX_STR));
                    size += sizeof(RBX_STR);
                    break;
                case RCX:
                    memcpy(buf + size, RCX_STR, sizeof(RCX_STR));
                    size += sizeof(RCX_STR);
                    break;
                case RDX:
                    memcpy(buf + size, RDX_STR, sizeof(RDX_STR));
                    size += sizeof(RDX_STR);
                    break;
                default: END_DIS_AND_RETURN UNKNOWN_REG_DISASM_ERR;
            }
            cur_cmd++;
            break;
        case PUSH_VAL:
            memcpy(buf, PUSH_STR, sizeof(PUSH_STR));
            size = sizeof(PUSH_STR);
            cur_cmd++;
            buf[size] = ' ';
            size++;
            if (cur_cmd + sizeof(double) > file_in_data + file_in_size) {
                END_DIS_AND_RETURN NO_HALT_PROC_ERR;
            }
            // -2 to keep space for "\r\n" in the end
            int n = snprintf(buf + size, sizeof(buf) - size - 2, "%d", *((double *)cur_cmd));
            size += (n < sizeof(buf) - size - 2 ? n : sizeof(buf) - size - 2);
            cur_cmd += sizeof(double);
            break;
        case PUSH_REG:
            memcpy(buf, PUSH_STR, sizeof(PUSH_STR));
            size = sizeof(PUSH_STR);
            cur_cmd++;
            buf[size] = ' ';
            size++;
            switch (*cur_cmd) {
                case RAX:
                    memcpy(buf + size, RAX_STR, sizeof(RAX_STR));
                    size += sizeof(RAX_STR);
                    break;
                case RBX:
                    memcpy(buf + size, RBX_STR, sizeof(RBX_STR));
                    size += sizeof(RBX_STR);
                    break;
                case RCX:
                    memcpy(buf + size, RCX_STR, sizeof(RCX_STR));
                    size += sizeof(RCX_STR);
                    break;
                case RDX:
                    memcpy(buf + size, RDX_STR, sizeof(RDX_STR));
                    size += sizeof(RDX_STR);
                    break;
                default: END_DIS_AND_RETURN UNKNOWN_REG_DISASM_ERR;
            }
            cur_cmd++;
            break;
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
