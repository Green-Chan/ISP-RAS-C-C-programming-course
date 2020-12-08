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

        // =========== read expression ============
        expression *expr_tree = NULL;
        const char *err_pos = NULL;
        printf ("read result: %d\n", read_expression(buf, &expr_tree, &err_pos));

        printf("err_idx: %d\n\n", (int) (err_pos - buf));

        // =========== print expression ============
        printf("expresion:\n");
        print_expr_tree_stdout(expr_tree);
        printf("\n\n");

        static unsigned char num = 0;
        char buf[50];
        size_t len = sprintf(buf, "output_files/graph%d", (int)num);
        assert(len < 50);
        buf[len] = '\0';
        print_expr_tree_graph(expr_tree, buf);

        len = sprintf(buf, "output_files/formula%d", (int)num);
        assert(len < 50);
        buf[len] = '\0';
        print_expr_formula(expr_tree, buf);

        // =========== print copied expression ============
        expression *copy_tree = copy_expression(expr_tree);
        if (copy_tree == (expression *) 1) {
            printf("No memory\n\n");
        } else {
            if (expr_tree != NULL) {
                expr_tree->type = 13;
            }
            destruct_expression(expr_tree);
            expr_tree = copy_tree;
            printf("copied expression:\n");
            print_expr_tree_stdout(expr_tree);
            printf("\n\n");

        // =========== simplify expression and print ============
            expr_tree = simplify_expression(expr_tree, 1e-5);

            printf("simplified expression:\n");
            print_expr_tree_stdout(expr_tree);
            printf("\n\n");

            len = sprintf(buf, "output_files/simp_graph%d", (int)num);
            assert(len < 50);
            buf[len] = '\0';
            print_expr_tree_graph(expr_tree, buf);

            len = sprintf(buf, "output_files/simp_formula%d", (int)num);
            assert(len < 50);
            buf[len] = '\0';
            print_expr_formula(expr_tree, buf);
        }

        // =========== differentiate (simplified) expression and print ============
        expression *diff_tree = differentiate_expression(expr_tree, 'x');
        if (diff_tree == (expression *) 1) {
            printf("No memory");
        } else {
            printf("differentiated expression:\n");
            print_expr_tree_stdout(diff_tree);
            printf("\n\n");

            len = sprintf(buf, "output_files/diff_graph%d", (int)num);
            assert(len < 50);
            buf[len] = '\0';
            print_expr_tree_graph(diff_tree, buf);

            len = sprintf(buf, "output_files/diff_formula%d", (int)num);
            assert(len < 50);
            buf[len] = '\0';
            print_expr_formula(diff_tree, buf);

        // =========== simplify (differentiated) expression and print ============
            diff_tree = simplify_expression(diff_tree, 1e-5);

            printf("simplified differentiated expression:\n");
            print_expr_tree_stdout(diff_tree);
            printf("\n\n");

            len = sprintf(buf, "output_files/simp_diff_graph%d", (int)num);
            assert(len < 50);
            buf[len] = '\0';
            print_expr_tree_graph(diff_tree, buf);

            len = sprintf(buf, "output_files/simp_diff_formula%d", (int)num);
            assert(len < 50);
            buf[len] = '\0';
            print_expr_formula(diff_tree, buf);
        }

        destruct_expression(expr_tree);
        destruct_expression(diff_tree);
        num++;
    }

}
