#include <stdio.h>
#include <assert.h>

#define PRINT_STACK_TYPE(val) printf("%d", val)
#define STACK_TYPE int
#include "stack.h"
#undef STACK_TYPE
#undef PRINT_STACK_TYPE

#define PRINT_STACK_TYPE(val) printf("%f", val)
#define STACK_TYPE double
#include "stack.h"
#undef STACK_TYPE
#undef PRINT_STACK_TYPE

int main() {
    TEMPLATE(int, stack) si1, si2;
    TEMPLATE(double, stack) sd1;
    construct_stack(int, &si1);
    construct_stack(int, &si2);
    construct_stack(double, &sd1);

    const int bign = 10000;

    for (int i = 0; i < bign; i++) {
        /*
        if (i == 10) {
            TEMPLATE(int, stack_dump)(&si1);
        }
        */
        push_stack(int, &si1, i);
        push_stack(int, &si2, i);
    }
    assert(stack_size(int, &si1) == bign);

    for (int i = bign - 1; i >= 0; i--) {
        assert(pop_stack(int, &si1) == i);
    }
    assert(stack_size(int, &si1) == 0);
    assert(is_empty_stack(int, &si1) == true);
    assert(is_empty_stack(int, &si2) == false);

    reserve_stack(int, &si1, 0);
    assert(stack_capacity(int, &si1) == 0);

    destruct_stack(int, &si1);
    destruct_stack(int, &si2);
    destruct_stack(double, &sd1);

    printf("ALL TESTS PASSED\n");
}
