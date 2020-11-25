#include <stdio.h>
#include <assert.h>

#include "expression_tree.h"

int main() {
    char buf[100];
    int c = 0;
    while(c != EOF) {
        size_t i = 0;
        while (c = getchar(), c != '\n') {
            if (c == EOF) {
                return 0;
            }
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

        static unsigned char num = 0;
        char buf[15];
        size_t len = sprintf(buf, "graph%d", (int)num);
        assert(len < 15);
        buf[len] = '\0';
        print_expr_tree_graph(expr_tree, buf);

        len = sprintf(buf, "formula%d", (int)num++);
        assert(len < 15);
        buf[len] = '\0';
        print_expr_formula(expr_tree, buf);

        destruct_expression(expr_tree);
    }
}
