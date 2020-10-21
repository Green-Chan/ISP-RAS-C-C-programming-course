#include <stdio.h>
#include <assert.h>
#include <math.h>

#define PRINT_STACK_TYPE(val) printf("%d", val)
#define STACK_TYPE int
#include "stack.h"
#undef STACK_TYPE
#undef PRINT_STACK_TYPE
#undef STACK_TYPE_CMP

#define STACK_TYPE double
#define STACK_TYPE_CMP(a, b) fabs(a - b) < 1e-5
#include "stack.h"
#undef STACK_TYPE
#undef PRINT_STACK_TYPE
#undef STACK_TYPE_CMP

int main() {
    TEMPLATE(int, stack) si1, si2;
    TEMPLATE(double, stack) sd1;
    construct_stack(int, &si1);
    construct_stack(int, &si2);
    construct_stack(double, &sd1);

    const int bign = 10000;

    for (int i = 0; i < bign; i++) {
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
