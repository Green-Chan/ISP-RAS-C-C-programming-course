///------------------------------------------------------------------------------------
//! @file
//! To use stack from this header you should define STACK_TYPE (the type of elements, stack will store),
//! then include this file. If you want to use stacks with different types, undef STACK_TYPE after
//! including this file, then define another STACK_TYPE and include this file again (repeat as many times
//! as necessary).
//!
//! To declare new stacks write <CODE> TEMPLATE(type, stack) s1, s2; </CODE> where @c type is
//! the type of elements, stack will store. Always initialize stacks before using (use @c construct_stack)
//! and destroy them when you don't need them (use @c destruct_stack).
//!
//! To call stack method for some stack write <CODE> <method_name>(type, ptr, [args]) </CODE>
//! where @c type is the type of elements, the stack stores, @c ptr is a pointer to the stack,
//! and <CODE> [args] </CODE> are the other arguments of the method (if there are some).
//!
//! See usage example in run_tests.cpp
//!
///------------------------------------------------------------------------------------




#ifndef STACK_COMMON

#define TEMPLATE_IN(type, name) name ## _ ## type
#define TEMPLATE(type, name) TEMPLATE_IN(type, name)
#define TO_STRING_IN(s)  # s
#define TO_STRING(s) TO_STRING_IN(s)

#endif

struct TEMPLATE(STACK_TYPE, stack_s)
{
    ssize_t size;
    ssize_t capacity;
    STACK_TYPE *data;
};

typedef struct TEMPLATE(STACK_TYPE, stack_s) TEMPLATE(STACK_TYPE, stack);

#ifndef STACK_COMMON
#define STACK_COMMON

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum stack_error_type {
    STACK_OK = 0,
    NULL_POINTER,
    NEGATIVE_SIZE,
    NEGATIVE_CAPACITY,
    SIZE_GREATER_THAN_CAPACITY,
    NULL_DATA_AND_POSITIVE_CAPACITY,
    NULL_CAPACITY_AND_NOTNULL_DATA,
    EMPTY_STACK
} stack_error_type;


///------------------------------------------------------------------------------------
//! Gets error name by error number
//!
//! @param [in] error   error number
//!
//! @return error name
//!
///------------------------------------------------------------------------------------

const char *stack_error_name(stack_error_type error) {
    switch(error) {
        case STACK_OK: return "STACK_OK";
        case NULL_POINTER: return "NULL_POINTER";
        case NEGATIVE_SIZE: return "NEGATIVE_SIZE";
        case NEGATIVE_CAPACITY: return "NEGATIVE_CAPACITY";
        case SIZE_GREATER_THAN_CAPACITY: return "SIZE_GREATER_THAN_CAPACITY";
        case NULL_DATA_AND_POSITIVE_CAPACITY: return "NULL_DATA_AND_POSITIVE_CAPACITY";
        case NULL_CAPACITY_AND_NOTNULL_DATA: return "NULL_CAPACITY_AND_NOTNULL_DATA";
        case EMPTY_STACK: return "POP_FROM_EMPTY_STACK";
        default: break;
    }
    return "UNKNOWN_ERROR";
}

#define standart_stack_assert(type, thou, file, line)                                            \
do {                                                                                             \
    stack_error_type error = TEMPLATE(type, stack_not_ok)(thou);                                 \
    if (error != STACK_OK) {                                                                     \
        printf("%s(%d): %s\n", file, line, stack_error_name(TEMPLATE(type, stack_not_ok)(thou)));\
        if (error == NULL_POINTER) {                                                             \
            assert(!"Got NULL instead of stack");                                                \
        } else {                                                                                 \
            assert(!"Stack is not ok");                                                          \
        }                                                                                        \
    }                                                                                            \
} while(0)

//! The capacity, stack will have after push in stack with zero capacity
#define FIRST_STACK_CAPACITY 1


///------------------------------------------------------------------------------------
//! Checks if stack fields have valid values. If it is not, returns error number.
//!
//! @param[in]     type   type of the stack elements
//! @param[in]     thou   pointer to the stack
//!
//! @return 0 if stack is ok, error number otherwise.
//!
///------------------------------------------------------------------------------------
#define stack_not_ok(type, thou) TEMPLATE(STACK_TYPE, stack_not_ok) (thou)

///------------------------------------------------------------------------------------
//! Constructs stack: puts valid values in the stack fields, that corresponds to an empty stack.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//!
///------------------------------------------------------------------------------------
#define construct_stack(type, thou) TEMPLATE(type, construct_stack) (thou, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Destructs stack: free allocated memory and puts invalid values in the stack fields.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//!
///------------------------------------------------------------------------------------
#define destruct_stack(type, thou) TEMPLATE(type, destruct_stack) (thou, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Checks if stack is empty.
//!
//! @param[in]     thou   pointer to the stack
//!
//! @return 1 if stack is empty, 0 if it is not
//!
///------------------------------------------------------------------------------------
#define is_empty_stack(type, thou) TEMPLATE(type, is_empty_stack) (thou, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Pops the last element out of stack.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//!
//! @return popped element
//!
///------------------------------------------------------------------------------------
#define pop_stack(type, thou) TEMPLATE(type, pop_stack) (thou, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Reads the last element of stack without popping it.
//!
//! @param[in]     type   type of the stack elements
//! @param[in]     thou   pointer to the stack
//!
//! @return last element
//!
///------------------------------------------------------------------------------------
#define read_stack(type, thou) TEMPLATE(type, read_stack) (thou, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Pushes new element in stack. Doubles capacity and reallocates memory if necessary.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//! @param[in]     elem   the new element
//!
//! @return 0 on success, 1 otherwise
//!
//! @note If 1 is returned, it means, that the memory cannot be allocated/reallocated.
//!       You can try to reserve needed amount of memory using @c stack_capacity and then push element.
//!
///------------------------------------------------------------------------------------
#define push_stack(type, thou, elem) TEMPLATE(type, push_stack) (thou, elem, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Gets size of the stack (number of elements in it).
//!
//! @param[in]     type   type of the stack elements
//! @param[in]     thou   pointer to the stack
//!
//! @return size of the stack
//!
///------------------------------------------------------------------------------------
#define stack_size(type, thou) TEMPLATE(type, stack_size) (thou, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Gets capacity of the stack (how much elements it can hold without allocation/reallocation).
//!
//! @param[in]     type   type of the stack elements
//! @param[in]     thou   pointer to the stack
//!
//! @return capacity of the stack
//!
///------------------------------------------------------------------------------------
#define stack_capacity(type, thou) TEMPLATE(type, stack_capacity) (thou, __FILE__, __LINE__)

///------------------------------------------------------------------------------------
//! Sets the capacity of the stack. Can be used either to allocate more memory or to free memory you don't need.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//! @param[in]     new_capacity   the new capacity of the stack
//!
//! @return 0 on success, 1 otherwise
//!
//! @note If 1 is returned, it means, that the memory cannot be allocated/reallocated.
//!       You can try to reserve less memory.
//!
///------------------------------------------------------------------------------------
#define reserve_stack(type, thou, new_capacity) TEMPLATE(type, reserve_stack) (thou, new_capacity, __FILE__, __LINE__)

#endif

stack_error_type TEMPLATE(STACK_TYPE, stack_not_ok) (TEMPLATE(STACK_TYPE, stack) *thou) {
    if (thou == NULL) { return NULL_POINTER; }
    if (thou->size < 0) { return NEGATIVE_SIZE; }
    if (thou->capacity < 0) { return NEGATIVE_CAPACITY; }
    if (thou->capacity < thou->size) { return SIZE_GREATER_THAN_CAPACITY; }
    if (thou->data == NULL && thou->capacity > 0) { return NULL_DATA_AND_POSITIVE_CAPACITY; }
    if (thou->data != NULL && thou->capacity == 0) { return NULL_CAPACITY_AND_NOTNULL_DATA; }
    return STACK_OK;
}

void TEMPLATE(STACK_TYPE, construct_stack) (TEMPLATE(STACK_TYPE, stack) *thou, const char *file, int line) {
    if (thou == NULL) {
        printf("%s(%d): NULL_POINTER\n", file, line);
        assert(!"Got NULL instead of stack");
    }
    thou->size = 0;
    thou->capacity = 0;
    thou->data = NULL;
}

void TEMPLATE(STACK_TYPE, destruct_stack) (TEMPLATE(STACK_TYPE, stack) *thou, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    free(thou->data);
    thou->data = (STACK_TYPE *)1; // POISON
    thou->size = -666; // POISON
    thou->capacity = -666; // POISON
}

bool TEMPLATE(STACK_TYPE, is_empty_stack) (TEMPLATE(STACK_TYPE, stack) *thou, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    return thou->size == 0;
}

STACK_TYPE TEMPLATE(STACK_TYPE, pop_stack) (TEMPLATE(STACK_TYPE, stack) *thou, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    if (is_empty_stack(STACK_TYPE, thou)) {
        printf("%s(%d): POP_FROM_EMPTY_STACK\n", file, line);
        assert(!"Pop from empty stack");
    }
    return thou->data[--thou->size];
}

STACK_TYPE TEMPLATE(STACK_TYPE, read_stack) (TEMPLATE(STACK_TYPE, stack) *thou, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    if (is_empty_stack(STACK_TYPE, thou)) {
        printf("%s(%d): EMPTY_STACK\n", file, line);
        assert(!"Read from empty stack");
    }
    return thou->data[thou->size - 1];
}

int TEMPLATE(STACK_TYPE, push_stack) (TEMPLATE(STACK_TYPE, stack) *thou, STACK_TYPE elem, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    if (thou->capacity == 0) {
        thou->data = calloc(FIRST_STACK_CAPACITY > 0 ? FIRST_STACK_CAPACITY : 1, sizeof(elem));
        if (thou->data == NULL) {
            return 1;
        }
        thou->capacity = FIRST_STACK_CAPACITY > 0 ? FIRST_STACK_CAPACITY : 1;
    } else if (thou->size == thou->capacity) {
        STACK_TYPE *new_data = realloc(thou->data, thou->capacity * 2 * sizeof(elem));
        if (new_data == NULL) {
            return 1;
        }
        memset(new_data + thou->capacity, 0, thou->capacity);
        thou->data = new_data;
        thou->capacity *= 2;
    }
    thou->data[thou->size++] = elem;
    return 0;
}

ssize_t TEMPLATE(STACK_TYPE, stack_size) (TEMPLATE(STACK_TYPE, stack) *thou, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    return thou->size;
}

ssize_t TEMPLATE(STACK_TYPE, stack_capacity) (TEMPLATE(STACK_TYPE, stack) *thou, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    return thou->capacity;
}

int TEMPLATE(STACK_TYPE, reserve_stack) (TEMPLATE(STACK_TYPE, stack) *thou, ssize_t new_capacity, const char *file, int line) {
    standart_stack_assert(STACK_TYPE, thou, file, line);
    if (new_capacity < thou->size) {
        printf("%s(%d): SIZE_GREATER_THAN_CAPACITY\n", file, line);
        assert(!"Invalid new_capacity");
    }
    if (thou->capacity != new_capacity) {
        if (thou->capacity == 0) {
            thou->data = calloc(new_capacity, sizeof(STACK_TYPE));
            if (thou->data == NULL) {
                return 1;
            }
        } else if (new_capacity == 0) {
            free(thou->data);
            thou->data = NULL;
        } else {
            STACK_TYPE *new_data = realloc(thou->data, new_capacity);
            if (new_data == NULL) {
                return 1;
            }
            if (new_capacity > thou->capacity) {
                memset(new_data + thou->capacity, 0, new_capacity - thou->capacity);
                thou->data = new_data;
            }
            assert(thou->data == new_data);
        }
        thou->capacity = new_capacity;
    }
    return 0;
}

