///------------------------------------------------------------------------------------
//! @file
//! To use stack from this header you should define STACK_TYPE (the type of elements, stack will store),
//! then include this file. If you want to use stacks with different types, undef STACK_TYPE after
//! including this file, then define another STACK_TYPE and include this file again (repeat as many times
//! as necessary). STACK_TYPE should be one word (use typedef if it isn't).
//!
//! To declare new stacks write <CODE> TEMPLATE(type, stack) s1, s2; </CODE> where @c type is
//! the type of elements, stack will store. Always initialize stacks before using (use @c construct_stack)
//! and destroy them when you don't need them (use @c destruct_stack).
//!
//! To call stack method for some stack write <CODE> <method_name>(type, ptr, [args]) </CODE>
//! where @c type is the type of elements, the stack stores, @c ptr is a pointer to the stack,
//! and <CODE> [args] </CODE> are the other arguments of the method (if there are some).
//!
//! Stack has several levels of security:
//!
//! If DEBUG=0, stack does not check itself;
//!
//! If DEBUG=1, stack checks if stack fields store valid values (size \f$ \leq \f$ capacity,
//! data \f$ \neq \f$ NULL if capacity \f$ > \f$ 0, etc.). If they are not, program prints current stack and
//! asserts. Stack also checks if reserved but not used elements are equal STACK_TYPE_POISON.
//! If you not define STACK_TYPE_POISON, it is 0, so if STACK_TYPE cannot be compared with 0, the
//! program will not compile.
//! You should define PRINT_STACK_TYPE, if you want to see the values of stack elements, when
//! current stack is printed.
//! If you want some specific comparison for STACK_TYPE, you should define STACK_TYPE_CMP.
//!
//! If DEBUG=2, stack do checks as when DEBUG=1 and also checks canaries that surround struct stack
//! and the buffer with elements. (In this code hungry_cat written instead of canary just for fun.
//! When canary is okay, it is flying and cat is hungry. When canary feels bad, cat can eat it.)
//!
//! If DEBUG=3, stack do checks as when DEBUG=2 and also checks if the hash of stack is equal to the
//! hash counted the last time the stack has been changed.
//!
//! See stack usage example in run_tests.cpp
//!
///------------------------------------------------------------------------------------

#ifndef DEBUG
    #define DEBUG 0
#endif

#if DEBUG > 0
    #ifndef STACK_TYPE_POISON
        #define STACK_TYPE_POISON 0
    #endif

    #ifndef PRINT_STACK_TYPE
        #define PRINT_STACK_TYPE(val) printf("PRINT_STACK_TYPE is not defined");
    #endif

    #ifndef STACK_TYPE_CMP
        #define STACK_TYPE_CMP(a, b) a == b
    #endif
#endif


#ifndef STACK_COMMON

#include <stdlib.h>

#define TEMPLATE_IN(type, name) name ## _ ## type
#define TEMPLATE(type, name) TEMPLATE_IN(type, name)
#define TO_STRING_IN(s)  # s
#define TO_STRING(s) TO_STRING_IN(s)

#endif

struct TEMPLATE(STACK_TYPE, stack_s)
{
    #if DEBUG > 1
        unsigned long long first_hungry_cat;
    #endif
    ssize_t size;
    ssize_t capacity;
    STACK_TYPE *data;
    #if DEBUG > 0
        //where it was constructed (for dump)
        const char *stack_pointer_name;
        const char *born_file;
        int         born_line;
        // where was the call of current stack_method (for errors)
        // "NULL" and 0 if method was not called or was not called properly
        const char *call_file;
        int         call_line;
    #endif // DEBUG > 0
    #if DEBUG > 2
        unsigned long long stack_hash;
    #endif
    #if DEBUG > 1
        unsigned long long second_hungry_cat;
    #endif
};

typedef struct TEMPLATE(STACK_TYPE, stack_s) TEMPLATE(STACK_TYPE, stack);

#ifndef STACK_COMMON
#define STACK_COMMON

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if DEBUG > 1
    const unsigned long long HUNGRY_CAT_VAL = 0xCA715172EA7BEEF1;
                                       // CAT IS WANT TO EAT BEEF !
#endif

#if DEBUG > 2
    unsigned long long hash_mod = 257; // prime

    unsigned long long stack_hash(unsigned char *ptr, size_t size) {
        unsigned long long result = 0;
        unsigned long long mod_pov = 1;
        for (size_t i = 0; i < size; i++) {
            result += ptr[i] * mod_pov;
            mod_pov *= hash_mod;
        }
        return result;
    }

#endif // DEBUG > 2

#if DEBUG > 0
    typedef enum stack_error_type {
        STACK_OK = 0,
        NULL_POINTER,
        NEGATIVE_SIZE,
        NEGATIVE_CAPACITY,
        SIZE_GREATER_THAN_CAPACITY,
        NULL_DATA_AND_POSITIVE_CAPACITY,
        NULL_CAPACITY_AND_NOTNULL_DATA,
        EMPTY_STACK,
        EMPTY_DATA_VALUE_NOT_POISON,
        FIRST_STRUCT_CAT_IS_FULL,
        SECOND_STRUCT_CAT_IS_FULL,
        FIRST_DATA_CAT_IS_FULL,
        SECOND_DATA_CAT_IS_FULL,
        BAD_HASH
    } stack_error_type;


///------------------------------------------------------------------------------------
//! Gets error name by error number
//!
//! @param [in] error   error number
//!
//! @return error name
//!
///------------------------------------------------------------------------------------

inline static const char *stack_error_name(stack_error_type error) {
    switch(error) {
        case STACK_OK: return "STACK_OK";
        case NULL_POINTER: return "NULL_POINTER";
        case NEGATIVE_SIZE: return "NEGATIVE_SIZE";
        case NEGATIVE_CAPACITY: return "NEGATIVE_CAPACITY";
        case SIZE_GREATER_THAN_CAPACITY: return "SIZE_GREATER_THAN_CAPACITY";
        case NULL_DATA_AND_POSITIVE_CAPACITY: return "NULL_DATA_AND_POSITIVE_CAPACITY";
        case NULL_CAPACITY_AND_NOTNULL_DATA: return "NULL_CAPACITY_AND_NOTNULL_DATA";
        case EMPTY_STACK: return "POP_FROM_EMPTY_STACK";
        case EMPTY_DATA_VALUE_NOT_POISON: return "EMPTY_DATA_VALUE_NOT_POISON";
        case FIRST_STRUCT_CAT_IS_FULL: return "FIRST_STRUCT_CAT_IS_FULL";
        case SECOND_STRUCT_CAT_IS_FULL : return "SECOND_STRUCT_CAT_IS_FULL";
        case FIRST_DATA_CAT_IS_FULL: return "FIRST_DATA_CAT_IS_FULL";
        case SECOND_DATA_CAT_IS_FULL: return "SECOND_DATA_CAT_IS_FULL";
        case BAD_HASH: return "BAD_HASH";
        default: break;
    }
    return "UNKNOWN_ERROR";
}

#define standart_stack_assert(type, thou, flag_before)                                           \
do {                                                                                             \
    stack_error_type error = TEMPLATE(type, stack_not_ok)(thou);                                 \
    if (error != STACK_OK) {                                                                     \
        if (flag_before) {                                                                       \
            printf("Stack was corrupted be someone else\n");                                     \
            printf("An error was detected in method, called in\n");                              \
        } else {                                                                                 \
            printf("Stack was corrupted in stack method, called in\n");                          \
        }                                                                                        \
        printf("%s(%d): %s\n", (thou)->call_file, (thou)->call_line,                             \
               stack_error_name(error));                                                         \
        TEMPLATE(STACK_TYPE, stack_dump) (thou);                                                 \
        if (error == NULL_POINTER) {                                                             \
            assert(!"Got NULL instead of stack");                                                \
        } else {                                                                                 \
            assert(!"Stack is not ok");                                                          \
        }                                                                                        \
    }                                                                                            \
} while(0)
#endif // DEBUG > 0

//! The capacity, stack will have after push in stack with zero capacity
#define FIRST_STACK_CAPACITY 1

#if DEBUG > 0
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

    #define file_line_save(thou) (thou)->call_file = __FILE__, (thou)->call_line = __LINE__
    #define file_line_dele(thou) (thou)->call_file = "NULL"; (thou)->call_line = 0

#endif // DEBUG > 0

///------------------------------------------------------------------------------------
//! Constructs stack: puts valid values in the stack fields, that corresponds to an empty stack.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//!
///------------------------------------------------------------------------------------
#if DEBUG > 0
    #define construct_stack(type, thou)\
    (file_line_save(thou), (thou)->stack_pointer_name = #thou,\
    (thou)->born_file = __FILE__, (thou)->born_line = __LINE__,\
    TEMPLATE(type, construct_stack) (thou) )
#else
    #define construct_stack(type, thou) TEMPLATE(type, construct_stack) (thou)
#endif
///------------------------------------------------------------------------------------
//! Destructs stack: free allocated memory and puts invalid values in the stack fields.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//!
///------------------------------------------------------------------------------------
#if DEBUG > 0
    #define destruct_stack(type, thou) \
    (file_line_save(thou), TEMPLATE(type, destruct_stack) (thou))
#else
    #define destruct_stack(type, thou) TEMPLATE(type, destruct_stack) (thou)
#endif

///------------------------------------------------------------------------------------
//! Checks if stack is empty.
//!
//! @param[in]     thou   pointer to the stack
//!
//! @return 1 if stack is empty, 0 if it is not
//!
///------------------------------------------------------------------------------------
#if DEBUG > 0
    #define is_empty_stack(type, thou) \
    (file_line_save(thou), TEMPLATE(type, is_empty_stack) (thou))
#else
    #define is_empty_stack(type, thou) TEMPLATE(type, is_empty_stack) (thou)
#endif

///------------------------------------------------------------------------------------
//! Pops the last element out of stack.
//!
//! @param[in]     type   type of the stack elements
//! @param[in,out] thou   pointer to the stack
//!
//! @return popped element
//!
///------------------------------------------------------------------------------------
#if DEBUG > 0
    #define pop_stack(type, thou) \
    (file_line_save(thou), TEMPLATE(type, pop_stack) (thou))
#else
    #define pop_stack(type, thou) TEMPLATE(type, pop_stack) (thou)
#endif

///------------------------------------------------------------------------------------
//! Reads the last element of stack without popping it.
//!
//! @param[in]     type   type of the stack elements
//! @param[in]     thou   pointer to the stack
//!
//! @return last element
//!
///------------------------------------------------------------------------------------
#if DEBUG > 0
    #define read_stack(type, thou) \
    (file_line_save(thou), TEMPLATE(type, read_stack) (thou))
#else
    #define read_stack(type, thou) TEMPLATE(type, read_stack) (thou)
#endif

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
#if DEBUG > 0
    #define push_stack(type, thou, elem) \
    (file_line_save(thou), TEMPLATE(type, push_stack) (thou, elem))
#else
    #define push_stack(type, thou, elem) TEMPLATE(type, push_stack) (thou, elem)
#endif

///------------------------------------------------------------------------------------
//! Gets size of the stack (number of elements in it).
//!
//! @param[in]     type   type of the stack elements
//! @param[in]     thou   pointer to the stack
//!
//! @return size of the stack
//!
///------------------------------------------------------------------------------------
#if DEBUG > 0
    #define stack_size(type, thou) \
    (file_line_save(thou), TEMPLATE(type, stack_size) (thou))
#else
    #define stack_size(type, thou) TEMPLATE(type, stack_size) (thou)
#endif

///------------------------------------------------------------------------------------
//! Gets capacity of the stack (how much elements it can hold without allocation/reallocation).
//!
//! @param[in]     type   type of the stack elements
//! @param[in]     thou   pointer to the stack
//!
//! @return capacity of the stack
//!
///------------------------------------------------------------------------------------
#if DEBUG > 0
    #define stack_capacity(type, thou) \
    (file_line_save(thou), TEMPLATE(type, stack_capacity) (thou))
#else
    #define stack_capacity(type, thou) TEMPLATE(type, stack_capacity) (thou)
#endif

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
#if DEBUG > 0
    #define reserve_stack(type, thou, new_capacity) \
    (file_line_save(thou), TEMPLATE(type, reserve_stack) (thou, new_capacity))
#else
    #define reserve_stack(type, thou, new_capacity) TEMPLATE(type, reserve_stack) (thou, new_capacity)
#endif

#endif // ndef STACK_COMMON

#if DEBUG > 2
    inline static unsigned long long TEMPLATE(STACK_TYPE, count_hash) (TEMPLATE(STACK_TYPE, stack) *thou) {
        assert(thou != NULL);
        unsigned long long saved_hash = thou->stack_hash;
        const char *saved_call_file = thou->call_file;
        int         saved_call_line = thou->call_line;
        thou->stack_hash = 0;
        thou->call_file = NULL;
        thou->call_line = 0;
        unsigned long long result = stack_hash((unsigned char *)thou, sizeof(*thou));
        if (thou->data != NULL) {
            result += stack_hash((unsigned char *)(((unsigned long long *)thou->data) - 1), thou->capacity * sizeof(STACK_TYPE) + 2 * sizeof(unsigned long long));
        }
        thou->stack_hash = saved_hash;
        thou->call_file = saved_call_file;
        thou->call_line = saved_call_line;
        return result;
    }
#endif // DEBUG > 2

#if DEBUG > 0
    inline static stack_error_type TEMPLATE(STACK_TYPE, stack_not_ok) (TEMPLATE(STACK_TYPE, stack) *thou) {
        if (thou == NULL) { return NULL_POINTER; }
        if (thou->size < 0) { return NEGATIVE_SIZE; }
        if (thou->capacity < 0) { return NEGATIVE_CAPACITY; }
        if (thou->capacity < thou->size) { return SIZE_GREATER_THAN_CAPACITY; }
        if (thou->data == NULL && thou->capacity > 0) { return NULL_DATA_AND_POSITIVE_CAPACITY; }
        if (thou->data != NULL && thou->capacity == 0) { return NULL_CAPACITY_AND_NOTNULL_DATA; }
        for (ssize_t i = thou->size; i < thou->capacity; i++) {
            if (!(STACK_TYPE_CMP(thou->data[i], STACK_TYPE_POISON))) { return EMPTY_DATA_VALUE_NOT_POISON; }
        }
        #if DEBUG > 1
            if (thou->first_hungry_cat != HUNGRY_CAT_VAL) { return FIRST_STRUCT_CAT_IS_FULL; }
            if (thou->second_hungry_cat != HUNGRY_CAT_VAL) { return SECOND_STRUCT_CAT_IS_FULL; }
            if (thou->data != NULL) {
                if (((unsigned long long *) thou->data)[-1] != HUNGRY_CAT_VAL) { return FIRST_DATA_CAT_IS_FULL; }
                if (((unsigned long long *)(thou->data + thou->capacity))[0] != HUNGRY_CAT_VAL) { return SECOND_DATA_CAT_IS_FULL; }
            }
        #endif
        #if DEBUG > 2
            if (thou->stack_hash != TEMPLATE(STACK_TYPE, count_hash)(thou)) { return BAD_HASH; }
        #endif
        return STACK_OK;
    }

    inline static void TEMPLATE(STACK_TYPE, stack_dump) (TEMPLATE(STACK_TYPE, stack) *thou);
#endif


#if DEBUG > 1

inline static void * TEMPLATE(STACK_TYPE, cat_alloc) (size_t capacity) {
    char *data = (char *)calloc(capacity * sizeof(STACK_TYPE) + 2 * sizeof(unsigned long long), 1);
    if (data == NULL) { return NULL; }
    ((unsigned long long *)data)[0] = HUNGRY_CAT_VAL;
    ((unsigned long long *)(data + sizeof(unsigned long long) + capacity * sizeof(STACK_TYPE)))[0] = HUNGRY_CAT_VAL;
    return (void *)(data + sizeof(unsigned long long));
}

inline static void * TEMPLATE(STACK_TYPE, cat_realloc) (void *data, size_t new_size) {
    data = (char *)realloc(((char *)data) - sizeof(unsigned long long), new_size + 2 * sizeof(unsigned long long));
    if (data == NULL) { return NULL; }
    ((unsigned long long *)data)[0] = HUNGRY_CAT_VAL;
    ((unsigned long long *)(data + sizeof(unsigned long long) + new_size))[0] = HUNGRY_CAT_VAL;
    return (void *)(data + sizeof(unsigned long long));
}

inline static void TEMPLATE(STACK_TYPE, cat_free) (void *data) {
    if (data != NULL) {
        free(((char *)data) - sizeof(unsigned long long));
    }
}

#endif // DEBUG > 1

inline static void TEMPLATE(STACK_TYPE, construct_stack) (TEMPLATE(STACK_TYPE, stack) *thou) {
    #if DEBUG > 0
        if (thou == NULL) {
            printf("%s(%d): NULL_POINTER\n", thou->call_file, thou->call_line);
            assert(!"Got NULL instead of stack");
        }
    #endif

    thou->size = 0;
    thou->capacity = 0;
    thou->data = NULL;
    #if DEBUG > 1
        thou->first_hungry_cat = HUNGRY_CAT_VAL;
        thou->second_hungry_cat = HUNGRY_CAT_VAL;
    #endif

    #if DEBUG > 2
        thou->stack_hash = TEMPLATE(STACK_TYPE, count_hash)(thou);
    #endif

    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, false);
        file_line_dele(thou);
    #endif
}

inline static void TEMPLATE(STACK_TYPE, destruct_stack) (TEMPLATE(STACK_TYPE, stack) *thou) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
    #endif

    #if DEBUG > 1
        TEMPLATE(STACK_TYPE, cat_free)(thou->data);
    #else
        free(thou->data);
    #endif

    #if DEBUG > 0
        thou->data = (STACK_TYPE *)1; // POISON
        thou->size = -666; // POISON
        thou->capacity = -666; // POISON
        file_line_dele(thou);
    #endif
}

inline static bool TEMPLATE(STACK_TYPE, is_empty_stack) (TEMPLATE(STACK_TYPE, stack) *thou) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
        file_line_dele(thou);
    #endif
    return thou->size == 0;
}

inline static STACK_TYPE TEMPLATE(STACK_TYPE, pop_stack) (TEMPLATE(STACK_TYPE, stack) *thou) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
        if (is_empty_stack(STACK_TYPE, thou)) {
            printf("%s(%d): POP_FROM_EMPTY_STACK\n", thou->call_file, thou->call_line);
            assert(!"Pop from empty stack");
        }
    #endif
    STACK_TYPE ret_val = thou->data[--thou->size];
    #if DEBUG > 0
        thou->data[thou->size] = STACK_TYPE_POISON;
        #if DEBUG > 2
            thou->stack_hash = TEMPLATE(STACK_TYPE, count_hash)(thou);
        #endif
        standart_stack_assert(STACK_TYPE, thou, false);
        file_line_dele(thou);
    #endif
    return ret_val;
}

inline static STACK_TYPE TEMPLATE(STACK_TYPE, read_stack) (TEMPLATE(STACK_TYPE, stack) *thou) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
        if (is_empty_stack(STACK_TYPE, thou)) {
            printf("%s(%d): POP_FROM_EMPTY_STACK\n", thou->call_file, thou->call_line);
            assert(!"Read from empty stack");
        }
    #endif
    return thou->data[thou->size - 1];
}

inline static int TEMPLATE(STACK_TYPE, push_stack) (TEMPLATE(STACK_TYPE, stack) *thou, STACK_TYPE elem) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
    #endif
    if (thou->capacity == 0) {
        #if DEBUG > 1
            thou->data = TEMPLATE(STACK_TYPE, cat_alloc)(FIRST_STACK_CAPACITY > 0 ? FIRST_STACK_CAPACITY : 1);
        #else
            thou->data = calloc(FIRST_STACK_CAPACITY > 0 ? FIRST_STACK_CAPACITY : 1, sizeof(elem));
        #endif
        if (thou->data == NULL) {
            #if DEBUG > 0
                standart_stack_assert(STACK_TYPE, thou, false);
                file_line_dele(thou);
            #endif
            return 1;
        }
        thou->capacity = FIRST_STACK_CAPACITY > 0 ? FIRST_STACK_CAPACITY : 1;
    } else if (thou->size == thou->capacity) {
        #if DEBUG > 1
            STACK_TYPE *new_data = TEMPLATE(STACK_TYPE, cat_realloc)(thou->data, thou->capacity * 2 * sizeof(elem));
        #else
            STACK_TYPE *new_data = realloc(thou->data, thou->capacity * 2 * sizeof(elem));
        #endif
        if (new_data == NULL) {
            #if DEBUG > 0
                standart_stack_assert(STACK_TYPE, thou, false);
                file_line_dele(thou);
            #endif
            return 1;
        }
        thou->data = new_data;
        thou->capacity *= 2;
    }
    thou->data[thou->size++] = elem;
    #if DEBUG > 0
        for (ssize_t i = thou->size; i < thou->capacity; i++ ) {
            thou->data[i] = STACK_TYPE_POISON;
        }
        #if DEBUG > 2
            thou->stack_hash = TEMPLATE(STACK_TYPE, count_hash)(thou);
        #endif
        standart_stack_assert(STACK_TYPE, thou, false);
        file_line_dele(thou);
    #endif
    return 0;
}

inline static ssize_t TEMPLATE(STACK_TYPE, stack_size) (TEMPLATE(STACK_TYPE, stack) *thou) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
        file_line_dele(thou);
    #endif
    return thou->size;
}

inline static ssize_t TEMPLATE(STACK_TYPE, stack_capacity) (TEMPLATE(STACK_TYPE, stack) *thou) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
        file_line_dele(thou);
    #endif
    return thou->capacity;
}

inline static int TEMPLATE(STACK_TYPE, reserve_stack) (TEMPLATE(STACK_TYPE, stack) *thou, ssize_t new_capacity) {
    #if DEBUG > 0
        standart_stack_assert(STACK_TYPE, thou, true);
        if (new_capacity < thou->size) {
            printf("%s(%d): SIZE_GREATER_THAN_CAPACITY\n", thou->call_file, thou->call_line);
            assert(!"Invalid new_capacity");
        }
    #endif
    if (thou->capacity != new_capacity) {
        if (thou->capacity == 0) {
            #if DEBUG > 1
                thou->data = TEMPLATE(STACK_TYPE, cat_alloc)(new_capacity);
            #else
                thou->data = calloc(new_capacity, sizeof(STACK_TYPE));
            #endif
            if (thou->data == NULL) {
                #if DEBUG > 0
                    standart_stack_assert(STACK_TYPE, thou, false);
                    file_line_dele(thou);
                #endif
                return 1;
            }
        } else if (new_capacity == 0) {
            #if DEBUG > 1
                TEMPLATE(STACK_TYPE, cat_free)(thou->data);
            #else
                free(thou->data);
            #endif
            thou->data = NULL;
        } else {
            #if DEBUG > 1
                STACK_TYPE *new_data = TEMPLATE(STACK_TYPE, cat_realloc)(thou->data, new_capacity);
            #else
                STACK_TYPE *new_data = realloc(thou->data, new_capacity);
            #endif
            if (new_data == NULL) {
                #if DEBUG > 0
                    standart_stack_assert(STACK_TYPE, thou, false);
                    file_line_dele(thou);
                #endif
                return 1;
            }
            thou->data = new_data;
        }
        thou->capacity = new_capacity;
    }
    #if DEBUG > 0
        for (ssize_t i = thou->size; i < thou->capacity; i++) {
            thou->data[i] = STACK_TYPE_POISON;
        }
        #if DEBUG > 2
            thou->stack_hash = TEMPLATE(STACK_TYPE, count_hash)(thou);
        #endif
        standart_stack_assert(STACK_TYPE, thou, false);
        file_line_dele(thou);
    #endif // DEBUG
    return 0;
}

#if DEBUG > 0
    inline static void TEMPLATE(STACK_TYPE, stack_dump) (TEMPLATE(STACK_TYPE, stack) *thou) {
        stack_error_type error = TEMPLATE(STACK_TYPE, stack_not_ok)(thou);
        printf("Stack[TYPE = %s] (%s) [0x%p]\n", TO_STRING(STACK_TYPE), stack_error_name(error), thou);
        if (error != NULL_POINTER) {
            printf("\"%s\"[%s(%d)]\n", thou->stack_pointer_name, thou->born_file, thou->born_line);
            printf("{\n");
            #if DEBUG > 1
                printf("  first_hungry_cat = 0x%I64x\n", thou->first_hungry_cat);
            #endif
            printf("  size = %Id\n", thou->size);
            printf("  size = %Id\n", thou->capacity);
            printf("  data[0x%p]\n", thou->data);
            if (thou->data != NULL && thou->capacity > 0) {
                printf("  {\n");
                #if DEBUG > 1
                    printf("  first_data_hungry_cat = 0x%I64x\n", ((unsigned long long *)thou->data)[-1]);
                #endif
                ssize_t i = 0;
                for (i = 0; i < thou->size && i < thou->capacity; i++) {
                    printf("    *[%Id] = ", i);
                    PRINT_STACK_TYPE(thou->data[i]);
                    if (STACK_TYPE_CMP(thou->data[i], STACK_TYPE_POISON)) {
                        printf(" // POISON!");
                    }
                    printf("\n");
                }
                for (; i < thou->capacity; i++) {
                    printf("     [%Id] = ", i);
                    PRINT_STACK_TYPE(thou->data[i]);
                    if (STACK_TYPE_CMP(thou->data[i], STACK_TYPE_POISON)) {
                        printf(" // POISON!");
                    }
                    printf("\n");
                }
                for (; i < thou->size; i++) {
                    printf("    *\n");
                }
                #if DEBUG > 1
                    printf("  second_data_hungry_cat = 0x%I64x\n", ((unsigned long long *)(thou->data + thou->capacity))[0]);
                #endif
                printf("  }\n");
            }
            #if DEBUG > 2
                printf("  stack_hash = %I64u\n", thou->stack_hash);
            #endif
            #if DEBUG > 1
                printf("  second_hungry_cat = 0x%I64x\n", thou->second_hungry_cat);
            #endif
            printf("}\n");
        }
    }
#endif // DEBUG > 0
