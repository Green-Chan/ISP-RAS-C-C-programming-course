#include <stdio.h>
#include "expression_tree.h"

int main() {
    char buf[100];
    int c = 0;
    size_t i = 0;
    while (c = getchar(), c != EOF) {
        buf[i++] = c;
    }
    buf[i] = 0;
    printf("%s\n", buf);

    expression *expr_tree = NULL;
    const char *err_pos = NULL;
    read_expression(buf, &expr_tree, &err_pos);
    printf("err_idx: %d\n\n", (int) (err_pos - buf));
    print_expr_tree_stdout(expr_tree);
    printf("\n\n");

    destruct_expression(expr_tree);
}
