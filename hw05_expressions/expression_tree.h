#ifndef EXPRESSION_TREE_GUARD

typedef struct expression {
    union {
       char operation;
       double number;
    };
    struct expression *children[2];
} expression;

enum {
    EXPR_OK = 0,
    EXPR_INVALID_ARGS,
    EXPR_INVALID_STR,
    EXPR_OUT_OF_MEM
};

int read_expression(const char *str, expression **expr_tree, const char **err_pos);

void destruct_expression(expression *expr_tree);

void print_expr_tree_stdout(expression *expr_tree);

int print_expr_tree_graph(expression *expr_tree, const char *name);

int print_expr_formula(expression *expr_tree, const char *name);

#define EXPRESSION_TREE_GUARD
#endif

