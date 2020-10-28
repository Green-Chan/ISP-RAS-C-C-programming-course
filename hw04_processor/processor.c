#include "processor.h"

#define STACK_TYPE double
#include "stack.h"
#undef STACK_TYPE

#include <string.h>
#include <math.h>
#include <windows.h>
#include <inttypes.h>
#include <stdio.h>


char *get_processor_file_error()
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

int header_error;

int check_header_proc(const char **cur_char, size_t file_size) {
    if (file_size < sizeof(header) || memcmp(*cur_char, header, sizeof(header)) != 0) {
        return 1;
    }
    (*cur_char) += sizeof(header);
    return 0;
}

#define END_PROC_AND_RETURN \
    destruct_stack(double, &proc_stack); \
    UnmapViewOfFile((LPCVOID)program); \
    CloseHandle(file_mapping); \
    CloseHandle(file_handle); \
    return

int process(const char *file_path)
{
    HANDLE file_handle = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        return FILE_PROC_ERR;
    }

    size_t program_size = GetFileSize(file_handle, NULL);
    if (program_size == INVALID_FILE_SIZE) {
        return FILE_PROC_ERR;
    }
    HANDLE file_mapping = CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (file_mapping == NULL) {
        return FILE_PROC_ERR;
    }
    const char*program = (const char*)MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, 0);
    if (program == NULL) {
        return FILE_PROC_ERR;
    }

    const char *cur_cmd = program;
    if (check_header_proc(&cur_cmd, program_size)) {
        UnmapViewOfFile((LPCVOID)program);
        CloseHandle(file_mapping);
        CloseHandle(file_handle);
        return HEADER_PROC_ERR;
    }

    TEMPLATE(double, stack) proc_stack;
    construct_stack(double, &proc_stack);
    double rax = NAN, rbx = NAN, rcx = NAN, rdx = NAN;

    while (cur_cmd < program + program_size) {
        switch (*cur_cmd) {
        case HALT:
            destruct_stack(double, &proc_stack);
            if (UnmapViewOfFile((LPCVOID)program) == 0) {
                return FILE_PROC_ERR;
            }
            if (CloseHandle(file_mapping) == 0) {
                return FILE_PROC_ERR;
            }
            if (CloseHandle(file_handle) == 0) {
                return FILE_PROC_ERR;
            }
            return PROC_SUCCESS;
        case IN_CMD:
            {
                double d;
                scanf("%lf", &d);
                if (push_stack(double, &proc_stack, d)) {
                    END_PROC_AND_RETURN OUT_OF_MEMORY_PROC_ERR;
                }
                cur_cmd++;
            }
            break;
        case OUT_CMD:
            if (is_empty_stack(double, &proc_stack)) {
                END_PROC_AND_RETURN POP_FROM_EMPTY_STACK_PROC_ERR;
            }
            printf("%lf\n", pop_stack(double, &proc_stack));
            cur_cmd++;
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            {
                if (stack_size(double, &proc_stack) < 2) {
                    END_PROC_AND_RETURN NOT_ENOUGH_ARGS_ON_STACK_PROC_ERR;
                }
                double arg2 = pop_stack(double, &proc_stack);
                double arg1 = pop_stack(double, &proc_stack);
                double result = NAN;
                switch (*cur_cmd) {
                    case ADD: result = arg1 + arg2; break;
                    case SUB: result = arg1 - arg2; break;
                    case MUL: result = arg1 * arg2; break;
                    case DIV: result = arg1 / arg2; break;
                    default: assert(!"impossible case");
                }

                int push_res = push_stack(double, &proc_stack, result);
                // It is always OK, 'cause we just poped two elements
                assert(push_res == 0);

                cur_cmd++;
            }
            break;
        case SQRT:
            {
                if (is_empty_stack(double, &proc_stack)) {
                    END_PROC_AND_RETURN NOT_ENOUGH_ARGS_ON_STACK_PROC_ERR;
                }
                double result = sqrt(pop_stack(double, &proc_stack));
                int push_res = push_stack(double, &proc_stack, result);
                // It is always OK, 'cause we just poped two elements
                assert(push_res == 0);
                cur_cmd++;
            }
            break;
        case POP:
            if (is_empty_stack(double, &proc_stack)) {
                END_PROC_AND_RETURN POP_FROM_EMPTY_STACK_PROC_ERR;
            }
            pop_stack(double, &proc_stack);
            cur_cmd++;
            break;
        case POP_REG:
            if (is_empty_stack(double, &proc_stack)) {
                END_PROC_AND_RETURN POP_FROM_EMPTY_STACK_PROC_ERR;
            }
            cur_cmd++;
            switch (*cur_cmd) {
                case RAX: rax = pop_stack(double, &proc_stack); break;
                case RBX: rbx = pop_stack(double, &proc_stack); break;
                case RCX: rcx = pop_stack(double, &proc_stack); break;
                case RDX: rdx = pop_stack(double, &proc_stack); break;
                default: END_PROC_AND_RETURN UNKNOWN_REGISTER_PROC_ERR;
            }
            cur_cmd++;
            break;
        case PUSH_VAL:
            cur_cmd++;
            if (cur_cmd + sizeof(double) > program + program_size) {
                END_PROC_AND_RETURN NO_HALT_PROC_ERR;
            }
            if (push_stack(double, &proc_stack, *((double *)cur_cmd))) {
                END_PROC_AND_RETURN OUT_OF_MEMORY_PROC_ERR;
            }
            cur_cmd += sizeof(double);
            break;
        case PUSH_REG:
            cur_cmd++;
            int push_res = 1;
            if (cur_cmd + 1 > program + program_size) {
                END_PROC_AND_RETURN NO_HALT_PROC_ERR;
            }
            switch (*cur_cmd) {
                case RAX: push_res = push_stack(double, &proc_stack, rax); break;
                case RBX: push_res = push_stack(double, &proc_stack, rbx); break;
                case RCX: push_res = push_stack(double, &proc_stack, rcx); break;
                case RDX: push_res = push_stack(double, &proc_stack, rdx); break;
                default: END_PROC_AND_RETURN UNKNOWN_REGISTER_PROC_ERR;
            }
            if (push_res) {
                END_PROC_AND_RETURN OUT_OF_MEMORY_PROC_ERR;
            }
            cur_cmd++;
            break;
        default: END_PROC_AND_RETURN UNKNOWN_COMMAND_PROC_ERR;
        }
    }

    END_PROC_AND_RETURN NO_HALT_PROC_ERR;
}

