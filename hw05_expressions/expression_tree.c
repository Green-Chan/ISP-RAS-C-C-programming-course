#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "expression_tree.h"

static void expr_skip_spaces(const char **str) {
    assert(str != NULL && *str != NULL);
    while (isspace(**str)) {
        (*str)++;
    }
}

void destruct_expression(expression *expr_tree) {
    if (expr_tree == NULL) {
        return;
    }
    #ifdef DEBUG
        expr_tree->type = 13;
    #endif // DEBUG
    for (size_t i = 0; i < sizeof(expr_tree->children) / sizeof(expr_tree->children[0]); i++) {
        destruct_expression(expr_tree->children[i]);
    }
    free(expr_tree);
}

expression *copy_expression(expression *expr_tree) {
    if (expr_tree == NULL) {
        return NULL;
    }
    expression *new_tree = calloc(1, sizeof(expression));
    if (new_tree == NULL) {
        return (expression *) 1;
    }
    *new_tree = *expr_tree;
    char mem_limit = 0;
    for (size_t i = 0; i < sizeof(expr_tree->children) / sizeof(expr_tree->children[0]); i++) {
        new_tree->children[i] = copy_expression(expr_tree->children[i]);
        if (new_tree->children[i] == (expression *) 1) {
            new_tree->children[i] = NULL;
            mem_limit = 1;
        }
    }
    if (mem_limit) {
        destruct_expression(new_tree);
        return (expression *) 1;
    }
    return new_tree;
}

#ifdef CREATE_TMP_EXPR_TREE_
    #error "That's a local define, it should not be defined"
#endif
#define CREATE_TMP_EXPR_TREE_ \
    expression *tmp_expr_tree = calloc(1, sizeof(expression)); \
    if (tmp_expr_tree == NULL) { \
        return EXPR_OUT_OF_MEM; \
    } \
    do {} while (0)

static int read_number(const char *str, expression **expr_tree, const char **err_pos) {
    if (str[0] == '-' || str[0] == '+') {
        *err_pos = str;
        return EXPR_INVALID_STR;
    }
    errno = 0;
    char *err_ptr;
    double readed_number = strtod(str, &err_ptr);
    if (err_ptr == str || errno == ERANGE) {
        *err_pos = err_ptr;
        return EXPR_INVALID_STR;
    } else {
        str = err_ptr;
        expr_skip_spaces(&str);
        *err_pos = str;

        CREATE_TMP_EXPR_TREE_;
        tmp_expr_tree->type = EXPR_NUMBER;
        tmp_expr_tree->number = readed_number;
        *expr_tree = tmp_expr_tree;

        return EXPR_OK;
    }
}

#ifdef CHECK_SYMBOL_
    #error "That's a local define, it should not be defined"
#endif
#define CHECK_SYMBOL_(symbol) \
    do { \
        if (str[0] == symbol) { \
            str++; \
        } else { \
            *err_pos = str; \
            return EXPR_INVALID_STR; \
        } \
    } while (0)

static int read_func(const char *str, char *operation, const char **err_pos) {

    #ifdef FUNC_END_
        #error "That's a local define, it should not be defined"
    #endif
    #define FUNC_END_(op) \
        do { \
            expr_skip_spaces(&str); \
            *operation = op; \
            *err_pos = str; \
            return EXPR_OK; \
        } while (0)

    switch (str[0]) {
    case 's':
        str++;
        CHECK_SYMBOL_('i');
        CHECK_SYMBOL_('n');
        FUNC_END_('s');
    case 'c':
        str++;
        CHECK_SYMBOL_('o');
        CHECK_SYMBOL_('s');
        FUNC_END_('c');
    case 'l':
        str++;
        CHECK_SYMBOL_('n');
        FUNC_END_('l');
    default:
        *err_pos = str;
        return EXPR_INVALID_STR;
    }

    #undef FUNC_END_
}

#undef CHECK_SYMBOL_

static int read_sumsub(const char *str, expression **expr_tree, const char **err_pos);


static int read_brackets(const char *str, expression **expr_tree, const char **err_pos) {
    expression *tmp_expr_tree;
    if (str[0] == '(') {
        str++;
        expr_skip_spaces(&str);
        int res = read_sumsub(str, &tmp_expr_tree, err_pos);
        if (res != EXPR_OK) {
            return res;
        }
        str = *err_pos;
        if (str[0] == ')') {
            str++;
            expr_skip_spaces(&str);
            *expr_tree = tmp_expr_tree;
            *err_pos = str;
            return EXPR_OK;
        }
    }
    *err_pos = str;
    return EXPR_INVALID_STR;
}

static int read_atom(const char *str, expression **expr_tree, const char **err_pos) {
    // May be that is just something in brackets?
    int res = read_brackets(str, expr_tree, err_pos);
    // Only if res == EXPR_INVALID_STR and *err_pos == str that is not something in brackets.
    // In other cases that is something in brackets, but something bad
    // and it cannot be any other atom
    if (!(res == EXPR_INVALID_STR && *err_pos == str)) {
        return res;
    }
    // It can be some other atom

    CREATE_TMP_EXPR_TREE_;
    // May be that is function?
    if (read_func(str, &tmp_expr_tree->operation, err_pos) == EXPR_OK) {
        // That could be only function, never another atom
        tmp_expr_tree->type = EXPR_OPERATION;
        int res = read_brackets(*err_pos, &tmp_expr_tree->children[1], err_pos);
        if (res != EXPR_OK) {
            destruct_expression(tmp_expr_tree);
            // err_pos is already set by read_brackets
            return res;
        }
        // Read function, read argument in brackets, everything is ok
        *expr_tree = tmp_expr_tree;
        // err_pos is already set
        return EXPR_OK;
    }

    // May be that is a variable?
    if (isalpha(str[0])) {
        // Number cannot start from alpha, so we can be sure, that it can be only a variable
        tmp_expr_tree->type = EXPR_VARIABLE;
        tmp_expr_tree->variable = str[0];
        str++;
        expr_skip_spaces(&str);
        *err_pos = str;
        *expr_tree = tmp_expr_tree;
        return EXPR_OK;
    }
    // We'll not need tmp_expr_tree if that is a number
    destruct_expression(tmp_expr_tree);

    // May be that is a number? That is the last hope, so we can just return the result
    return read_number(str, expr_tree, err_pos);
}

static int read_power(const char *str, expression **expr_tree, const char **err_pos) {
    expression *return_expr_tree;
    int res = read_atom(str, &return_expr_tree, err_pos);
    if (res != EXPR_OK) {
        return res;
    }
    str = *err_pos;
    expr_skip_spaces(&str);
    expression **last_atom = &return_expr_tree;
    while (str[0] == '^') {
        CREATE_TMP_EXPR_TREE_;
        tmp_expr_tree->type = EXPR_OPERATION;
        tmp_expr_tree->operation = '^';
        tmp_expr_tree->children[0] = *last_atom;
        *last_atom = tmp_expr_tree;
        last_atom = &tmp_expr_tree->children[1];

        str++;
        expr_skip_spaces(&str);

        res = read_atom(str, last_atom, err_pos);
        if (res != EXPR_OK) {
            destruct_expression(return_expr_tree);
            return res;
        }
        str = *err_pos;
        expr_skip_spaces(&str);
    }
    *expr_tree = return_expr_tree;
    return EXPR_OK;
}

#ifdef READ_SSMD_END_
    #error "That's a local define, it should not be defined"
#endif
#define READ_SSMD_END_(op1, op2, read_prev) \
    while (str[0] == op1 || str[0] == op2) { \
        CREATE_TMP_EXPR_TREE_; \
        tmp_expr_tree->type = EXPR_OPERATION; \
        tmp_expr_tree->operation = str[0]; \
        tmp_expr_tree->children[0] = return_expr_tree; \
        return_expr_tree = tmp_expr_tree; \
        str++; \
        expr_skip_spaces(&str); \
        int res = read_prev(str, &return_expr_tree->children[1], err_pos); \
        if (res != EXPR_OK) { \
            destruct_expression(return_expr_tree); \
            return res; \
        } \
        str = *err_pos; \
        expr_skip_spaces(&str); \
    } \
    *expr_tree = return_expr_tree; \
    return EXPR_OK

static int read_muldiv(const char *str, expression **expr_tree, const char **err_pos) {
    expression *return_expr_tree = NULL;
    int res = read_power(str, &return_expr_tree, err_pos);
    if (res != EXPR_OK) {
        return res;
    }
    str = *err_pos;
    expr_skip_spaces(&str);
    READ_SSMD_END_('*', '/', read_power);
}

static int read_sumsub(const char *str, expression **expr_tree, const char **err_pos) {
    expression *return_expr_tree = NULL;
    if (str[0] == '-') {
        return_expr_tree = calloc(1, sizeof(expression));
        if (return_expr_tree == NULL) {
            return EXPR_OUT_OF_MEM;
        }
        return_expr_tree->type = EXPR_NUMBER;
        return_expr_tree->number = 0;
        // Lets store unary minus as binary minus with zero first argument.
        // So put zero child and just go on, pretending that this zero was read
    } else {
        int res = read_muldiv(str, &return_expr_tree, err_pos);
        if (res != EXPR_OK) {
            return res;
        }
        str = *err_pos;
        expr_skip_spaces(&str);
    }
    READ_SSMD_END_('+', '-', read_muldiv);
}

#undef READ_SSMD_END_
#undef CREATE_TMP_EXPR_TREE_

int read_expression(const char *str, expression **expr_tree, const char **err_pos) {
    if (str == NULL || expr_tree == NULL) {
        return EXPR_INVALID_ARGS;
    }
    if (err_pos == NULL) {
        const char *tmp_err_pos;
        return read_sumsub(str, expr_tree, &tmp_err_pos);
    } else {
        return read_sumsub(str, expr_tree, err_pos);
    }
}

void print_expr_tree_stdout(expression *expr_tree) {
    static size_t height = 0;

    if (expr_tree == NULL) {
        return;
    }

    // If there is no child, nothing will be printed
    height++;
    print_expr_tree_stdout(expr_tree->children[0]);
    height--;

    for (size_t i = 0; i < height; i++) {
        printf("  ");
    }
    switch(expr_tree->type) {
    case EXPR_NUMBER:
        printf("%lf\n", expr_tree->number);
        break;
    case EXPR_VARIABLE:
        printf("%c\n", expr_tree->variable);
        break;
    case EXPR_OPERATION:
        if (expr_tree->operation == 's') {
            printf("sin\n");
        } else if (expr_tree->operation == 'c') {
            printf("cos\n");
        } else if (expr_tree->operation == 'l') {
            printf("ln\n");
        } else if (expr_tree->operation == '+' || expr_tree->operation == '-' || expr_tree->operation == '*' ||
                   expr_tree->operation == '/' || expr_tree->operation == '^') {
            printf("%c\n", expr_tree->operation);
        } else {
            printf("??\n");
        }
        break;
    default:
        printf("??\n");
    }

    // If there is no child, nothing will be printed
    height++;
    print_expr_tree_stdout(expr_tree->children[1]);
    height--;
}

static int print_expr_tree_graph_internal(expression *expr_tree, FILE *fout, size_t *node_number) {
    assert(expr_tree != NULL);

    #ifdef PRINT_
        #error "That's a local define, it should not be defined"
    #endif
    #define PRINT_(fmt, node_num, node_val) \
        do { \
            if (fprintf(fout, fmt, node_num, node_val) < 0) { \
                printf("ATTENTION\n"); \
                fclose(fout); \
                return 1; \
            } \
        } while (0)

    // node_number will change, but we will need it's current value
    size_t saved_node_number = *node_number;
    *node_number += 1;

    // Our parent (if exists) printed "parent_node->".
    // Print our node after it, then print our node with style,
    // then print "our_node->" for each existing child and call
    // print_expr_tree_graph_internal for it

    #ifdef PRINT_TREE_
        #error "That's a local define, it should not be defined"
    #endif
    #define PRINT_TREE_(part_of_fmt, style_str, node_val) \
        do { \
            PRINT_(part_of_fmt ";\r\n", saved_node_number, node_val); \
            PRINT_("\t" part_of_fmt style_str "\r\n", saved_node_number, node_val); \
            for (size_t i = 0; i < sizeof(expr_tree->children) / sizeof(expr_tree->children[0]); i++) { \
                if (expr_tree->children[i] != NULL) { \
                    PRINT_("\t" part_of_fmt "->", saved_node_number, node_val); \
                    print_expr_tree_graph_internal(expr_tree->children[i], fout, node_number); \
                } \
            } \
        } while (0)

    switch(expr_tree->type) {
    case EXPR_NUMBER:
        PRINT_TREE_("\"node %I64d:\n%lf\"", "[style=\"filled\",fillcolor=\"yellow\"]", expr_tree->number);
        break;
    case EXPR_VARIABLE:
        PRINT_TREE_("\"node %I64d:\n%c\"", "[style=\"filled\",fillcolor=\"lightblue\"]", expr_tree->variable);
        break;
    case EXPR_OPERATION:
        if (expr_tree->operation == 's') {
            PRINT_TREE_("\"node %I64d:\n%s\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", "sin");
        } else if (expr_tree->operation == 'c') {
            PRINT_TREE_("\"node %I64d:\n%s\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", "cos");
        } else if (expr_tree->operation == 'l') {
            PRINT_TREE_("\"node %I64d:\n%s\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", "ln");
        } else if (expr_tree->operation == '+' || expr_tree->operation == '-' || expr_tree->operation == '*' ||
                   expr_tree->operation == '/' || expr_tree->operation == '^') {
            PRINT_TREE_("\"node %I64d:\n%c\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", expr_tree->operation);
        } else {
            PRINT_TREE_("\"node %I64d:\n%s\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", "??");
        }
        break;
    default:
        PRINT_TREE_("\"node %I64d:\n%s\"", "[style=\"filled\",fillcolor=\"red\"]", "??");
    }

    return 0;

    #undef PRINT_
    #undef PRINT_TREE_
}

int print_expr_tree_graph(expression *expr_tree, const char *name) {
    size_t n = strlen(name);
    char buf[n + 5];
    memcpy(buf, name, n);
    memcpy(buf + n, ".dot", 5); // also '\0' symbol
    FILE *file_out = fopen(buf, "wb");
    if (file_out == NULL) {
        return 1;
    }
    buf[n] = '\0';
    if(fprintf(file_out, "digraph \"%s\" {\r\n", buf) < 0) {
        fclose(file_out);
        return 1;
    }

    if (expr_tree == NULL) {
        if (fprintf(file_out, "\tlabel = \"(Empty_tree)\";\r\n") < 0) {
            fclose(file_out);
            return 1;
        }
    } else {
        size_t node_num = 0;
        if (print_expr_tree_graph_internal(expr_tree, file_out, &node_num) != 0) {
            fclose(file_out);
            return 1;
        }
    }

    if (fprintf(file_out, "}\n") < 0) {
        fclose(file_out);
        return 1;
    }
    if (fclose(file_out) != 0) {
        return 1;
    }
    size_t cmd_len = strlen("dot \".dot\" -Tpdf -o \".pdf\"") + 1 + n + n;
    char command[cmd_len];
    size_t real_cmd_len = sprintf(command, "dot \"%s.dot\" -Tpdf -o \"%s.pdf\"", name, name);
    assert(real_cmd_len + 1 <= cmd_len);
    command[real_cmd_len] = '\0';
    system(command);

    real_cmd_len = sprintf(command, "start \"\" \"%s.pdf\"", name);
    assert(real_cmd_len + 1 <= cmd_len);
    command[real_cmd_len] = '\0';
    system(command);
    return 0;
}

static int print_expr_formula_internal(expression *expr_tree, FILE *fout) {
    #ifdef CHECK_RETURN_
        #error "That's a local define, it should not be defined"
    #endif
    #define CHECK_RETURN_(check) \
        do { \
            if (check) { \
                return 1; \
            } \
        } while (0)

    #ifdef PRINT_CHILD_
        #error "That's a local define, it should not be defined"
    #endif
    #define PRINT_CHILD_(idx) \
        do { \
            if (expr_tree->children[idx] == NULL) { \
                CHECK_RETURN_(fprintf(fout, "NOARG") < 0); \
            } else { \
                CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[idx], fout) != 0); \
            } \
        } while (0)

    #ifdef PRINT_ADD_SUB_CHILD_IN_BRACKETS_
        #error "That's a local define, it should not be defined"
    #endif
    #define PRINT_ADD_SUB_CHILD_IN_BRACKETS_(idx) \
        do { \
            if (expr_tree->children[idx] == NULL) { \
                CHECK_RETURN_(fprintf(fout, "NOARG") < 0); \
            } else if (expr_tree->children[idx]->type == EXPR_OPERATION && \
                       (expr_tree->children[idx]->operation == '+' || \
                        expr_tree->children[idx]->operation == '-')){ \
                CHECK_RETURN_(fprintf(fout, "(") < 0); \
                CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[idx], fout) != 0); \
                CHECK_RETURN_(fprintf(fout, ")") < 0); \
            } else { \
                CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[idx], fout) != 0); \
            } \
        } while (0)

    switch(expr_tree->type) {
    case EXPR_NUMBER:
        if (expr_tree->number < 0) {
            CHECK_RETURN_(fprintf(fout, "(%lf)", expr_tree->number) < 0);
        } else {
            CHECK_RETURN_(fprintf(fout, "%lf", expr_tree->number) < 0);
        }
        return 0;
    case EXPR_VARIABLE:
        CHECK_RETURN_(fprintf(fout, "%c", expr_tree->variable) < 0);
        return 0;
    case EXPR_OPERATION:
        switch(expr_tree->operation) {
        case '+':
            PRINT_CHILD_(0);
            CHECK_RETURN_(fprintf(fout, " + ") < 0);
            PRINT_CHILD_(1);
            return 0;
        case '-':
            PRINT_CHILD_(0);
            CHECK_RETURN_(fprintf(fout, " - ") < 0);
            PRINT_ADD_SUB_CHILD_IN_BRACKETS_(1);
            return 0;
        case '*':
            PRINT_ADD_SUB_CHILD_IN_BRACKETS_(0);
            CHECK_RETURN_(fprintf(fout, " \\cdot ") < 0);
            PRINT_ADD_SUB_CHILD_IN_BRACKETS_(1);
            return 0;
        case '/':
            CHECK_RETURN_(fprintf(fout, "\\frac{") < 0);
            PRINT_CHILD_(0);
            CHECK_RETURN_(fprintf(fout, "}{") < 0);
            PRINT_CHILD_(1);
            CHECK_RETURN_(fprintf(fout, "}") < 0);
            return 0;
        case '^':
            if (expr_tree->children[0] == NULL) {
                CHECK_RETURN_(fprintf(fout, "NOARG") < 0);
            } else if (expr_tree->children[0]->type == EXPR_OPERATION) {
                #ifdef FUNC_POWER_PRINT_
                    #error "That's a local define, it should not be defined"
                #endif
                #define FUNC_POWER_PRINT_(func) \
                    CHECK_RETURN_(fprintf(fout, func "^{") < 0); \
                    PRINT_CHILD_(1); \
                    CHECK_RETURN_(fprintf(fout, "}(") < 0); \
                    if (expr_tree->children[0]->children[1] == NULL) { \
                        CHECK_RETURN_(fprintf(fout, "NOARG") < 0); \
                    } else { \
                        CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[0]->children[1], fout) != 0); \
                    } \
                    CHECK_RETURN_(fprintf(fout, ")") < 0) \

                if (expr_tree->children[0]->operation == 's') {
                    FUNC_POWER_PRINT_("sin");
                    return 0;
                } else if (expr_tree->children[0]->operation == 'c') {
                    FUNC_POWER_PRINT_("cos");
                    return 0;
                } else if (expr_tree->children[0]->operation == 'l') {
                    FUNC_POWER_PRINT_("ln");
                    return 0;
                } else {
                    CHECK_RETURN_(fprintf(fout, "(") < 0);
                    CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[0], fout) != 0);
                    CHECK_RETURN_(fprintf(fout, ")") < 0);
                }
                #undef FUNC_POWER_PRINT_
            } else {
                CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[0], fout) != 0);
            }
            // If we are here, only first argument is printed
            CHECK_RETURN_(fprintf(fout, "^{") < 0);
            PRINT_CHILD_(1);
            CHECK_RETURN_(fprintf(fout, "}") < 0);
            return 0;

        #ifdef FUNC_PRINT_
            #error "That's a local define, it should not be defined"
        #endif
        #define FUNC_PRINT_(func) \
            CHECK_RETURN_(fprintf(fout, func "(") < 0); \
            PRINT_CHILD_(1); \
            CHECK_RETURN_(fprintf(fout, ")") < 0)
        case 's':
            FUNC_PRINT_("sin");
            return 0;
        case 'c':
            FUNC_PRINT_("cos");
            return 0;
        case 'l':
            FUNC_PRINT_("ln");
            return 0;
        #undef FUNC_PRINT_
        default:
            CHECK_RETURN_(fprintf(fout, "UNKNOWN") < 0);
            return 0;
        }
        assert(!"should not be here");
    default:
        CHECK_RETURN_(fprintf(fout, "UNKNOWN") < 0);
        return 0;
    }
    assert(!"should not be here");
    #undef CHECK_RETURN_
    #undef PRINT_CHILD_
    #undef PRINT_ADD_SUB_CHILD_IN_BRACKETS_
}

int print_expr_formula(expression *expr_tree, const char *name) {
    size_t n = strlen(name);
    char buf[n + 5];
    memcpy(buf, name, n);
    memcpy(buf + n, ".tex", 5); // also '\0' symbol
    FILE *file_out = fopen(buf, "wb");
    if (file_out == NULL) {
        return 1;
    }
    if (expr_tree == NULL) {
        if (fprintf(file_out, "\\documentclass{article}\r\n\\begin{document}\r\nNo expression\r\n\\end{document}\r\n") < 0) {
            fclose(file_out);
            return 1;
        }
    } else {
        if (fprintf(file_out, "\\documentclass{article}\r\n\\begin{document}\r\n$$") < 0) {
            fclose(file_out);
            return 1;
        }
        if (print_expr_formula_internal(expr_tree, file_out) != 0) {
            fclose(file_out);
            return 1;
        }
        if (fprintf(file_out, "$$\r\n\\end{document}\r\n") < 0) {
            fclose(file_out);
            return 1;
        }
    }
    if (fclose(file_out) != 0) {
        return 1;
    }
    size_t cmd_len = strlen("pdflatex -output-directory \".tex\"  >nul 2>nul") + 1 + n + n;
    char command[cmd_len];
    size_t real_cmd_len = 0;

    size_t directory_end = 0;
    for (size_t i = 0; i < n; i++) {
        if (name[i] == '/') {
            directory_end = i;
        }
    }
    if (directory_end == 0) {
        real_cmd_len += sprintf(command + real_cmd_len, "pdflatex ");
    } else {
        buf[directory_end] = '\0';
        real_cmd_len += sprintf(command + real_cmd_len, "pdflatex -output-directory %s ", buf);
    }

    real_cmd_len += sprintf(command + real_cmd_len, "\"%s.tex\" >nul 2>nul", name);

    assert(real_cmd_len + 1 <= cmd_len);
    command[real_cmd_len] = '\0';
    system(command);

    real_cmd_len = sprintf(command, "start \"\" \"%s.pdf\"", name);
    assert(real_cmd_len + 1 <= cmd_len);
    command[real_cmd_len] = '\0';
    system(command);
    return 0;
}

inline
static int comp_eps(double a, double b, double eps) {
    return fabs(a - b) <= eps;
}

expression *simplify_expression(expression *expr_tree, double eps) {
    if (expr_tree == NULL) {
        return NULL;
    }
    switch (expr_tree->type) {
    case EXPR_NUMBER:
        return expr_tree;
    case EXPR_VARIABLE:
        return expr_tree;
    case EXPR_OPERATION:
        #ifdef SIMPLIFY_TWO_ARGS_
            #error "That's a local define, it should not be defined"
        #endif
        #define SIMPLIFY_TWO_ARGS_ \
            do { \
                expr_tree->children[0] = simplify_expression(expr_tree->children[0], eps); \
                expr_tree->children[1] = simplify_expression(expr_tree->children[1], eps); \
                if (expr_tree->children[0] == NULL || expr_tree->children[1] == NULL) { \
                    return expr_tree; \
                } \
            } while (0)
        #ifdef RETURN_CHILD_
            #error "That's a local define, it should not be defined"
        #endif
        #define RETURN_CHILD_(idx) \
            expression *return_tree = expr_tree->children[idx]; \
            expr_tree->children[idx] = NULL; \
            destruct_expression(expr_tree); \
            return return_tree
        switch (expr_tree->operation) {
        case '+':
            SIMPLIFY_TWO_ARGS_;
            if (expr_tree->children[0]->type == EXPR_NUMBER) {
                if (expr_tree->children[1]->type == EXPR_NUMBER) {
                    expr_tree->children[1]->number += expr_tree->children[0]->number;
                    RETURN_CHILD_(1);
                }
                if (comp_eps(expr_tree->children[0]->number, 0, eps)) {
                    RETURN_CHILD_(1);
                }
            } else if (expr_tree->children[1]->type == EXPR_NUMBER && comp_eps(expr_tree->children[1]->number, 0, eps)) {
                RETURN_CHILD_(0);
            }
            return expr_tree;
        case '-':
            SIMPLIFY_TWO_ARGS_;
            if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (expr_tree->children[0]->type == EXPR_NUMBER) {
                    expr_tree->children[0]->number -= expr_tree->children[1]->number;
                    RETURN_CHILD_(0);
                }
                if (comp_eps(expr_tree->children[1]->number, 0, eps)) {
                    RETURN_CHILD_(0);
                }
            }
            return expr_tree;
        case '*':
            SIMPLIFY_TWO_ARGS_;
            if (expr_tree->children[0]->type == EXPR_NUMBER) {
                if (expr_tree->children[1]->type == EXPR_NUMBER) {
                    expr_tree->children[1]->number *= expr_tree->children[0]->number;
                    RETURN_CHILD_(1);
                }
                if (comp_eps(expr_tree->children[0]->number, 0, eps)) {
                    RETURN_CHILD_(0);
                }
                if (comp_eps(expr_tree->children[0]->number, 1, eps)) {
                    RETURN_CHILD_(1);
                }
            } else if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[1]->number, 0, eps)) {
                    RETURN_CHILD_(1);
                }
                if (comp_eps(expr_tree->children[1]->number, 1, eps)) {
                    RETURN_CHILD_(0);
                }
            }
            return expr_tree;
        case '/':
            SIMPLIFY_TWO_ARGS_;
            if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (expr_tree->children[0]->type == EXPR_NUMBER) {
                    expr_tree->children[0]->number /= expr_tree->children[1]->number;
                    RETURN_CHILD_(0);
                }
                if (comp_eps(expr_tree->children[1]->number, 1, eps)) {
                    RETURN_CHILD_(0);
                }
            } else if (expr_tree->children[0]->type == EXPR_NUMBER && comp_eps(expr_tree->children[0]->number, 0, eps)) {
                RETURN_CHILD_(0);
            }
            return expr_tree;
        case '^':
            SIMPLIFY_TWO_ARGS_;
            if (expr_tree->children[0]->type == EXPR_NUMBER) {
                if (expr_tree->children[1]->type == EXPR_NUMBER) {
                    expr_tree->children[0]->number = pow(expr_tree->children[0]->number, expr_tree->children[1]->number);
                    RETURN_CHILD_(0);
                }
                if (comp_eps(expr_tree->children[0]->number, 0, eps)) {
                    RETURN_CHILD_(0);
                }
                if (comp_eps(expr_tree->children[0]->number, 1, eps)) {
                    RETURN_CHILD_(0);
                }
            } else if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[1]->number, 0, eps)) {
                    expr_tree->children[1]->number = 1;
                    RETURN_CHILD_(1);
                }
                if (comp_eps(expr_tree->children[1]->number, 1, eps)) {
                    RETURN_CHILD_(0);
                }
            }
            return expr_tree;
        #ifdef FUNC_SIMP_
            #error "That's a local define, it should not be defined"
        #endif
        #define FUNC_SIMP_(func) \
            do { \
                expr_tree->children[1] = simplify_expression(expr_tree->children[1], eps); \
                if (expr_tree->children[1] == NULL) { \
                    return expr_tree; \
                } \
                if (expr_tree->children[1]->type == EXPR_NUMBER) { \
                    expr_tree->children[1]->number = func (expr_tree->children[1]->number); \
                    RETURN_CHILD_(1); \
                } \
            } while (0)
        case 's':
            FUNC_SIMP_(sin);
            return expr_tree;
        case 'c':
            FUNC_SIMP_(cos);
            return expr_tree;
        case 'l':
            FUNC_SIMP_(log);
            return expr_tree;
        #undef FUNC_SIMP_
        default:
            return expr_tree;
        }
        assert(!"should not be here");
        #undef SIMPLIFY_TWO_ARGS_
        #undef RETURN_CHILD_
    default:
        return expr_tree;
    }
    assert(!"should not be here");
}

expression *differentiate_expression(expression *expr_tree, char diff_var) {
    if (expr_tree == NULL) {
        return NULL;
    }

    expression *result_tree = NULL;

    #ifdef MAKE_NUM_NODE_
        #error "That's a local define, it should not be defined"
    #endif
    #define MAKE_NUM_NODE_(new_node_ptr, num) \
        new_node_ptr = calloc(1, sizeof(expression)); \
        if (new_node_ptr == NULL) { \
            destruct_expression(result_tree); \
            return (expression *) 1; \
        } \
        new_node_ptr->type = EXPR_NUMBER; \
        new_node_ptr->number = num

    #ifdef MAKE_OP_NODE_
        #error "That's a local define, it should not be defined"
    #endif
    #define MAKE_OP_NODE_(new_node_ptr, op) \
        new_node_ptr = calloc(1, sizeof(expression)); \
        if (new_node_ptr == NULL) { \
            destruct_expression(result_tree); \
            return (expression *) 1; \
        } \
        new_node_ptr->type = EXPR_OPERATION; \
        new_node_ptr->operation = op

    #ifdef MAKE_COPY_NODE_
        #error "That's a local define, it should not be defined"
    #endif
    #define MAKE_COPY_NODE_(new_node_ptr, old_node_ptr) \
        do { \
            new_node_ptr = copy_expression(old_node_ptr); \
            if (new_node_ptr == (expression *) 1) { \
                new_node_ptr = NULL; \
                destruct_expression(result_tree); \
                return (expression *) 1; \
            } \
        } while (0)

    #ifdef MAKE_DIFF_NODE_
        #error "That's a local define, it should not be defined"
    #endif
    #define MAKE_DIFF_NODE_(new_node_ptr, old_node_ptr) \
        do { \
            new_node_ptr = differentiate_expression(old_node_ptr, diff_var); \
            if (new_node_ptr == (expression *) 1) { \
                new_node_ptr = NULL; \
                destruct_expression(result_tree); \
                return (expression *) 1; \
            } \
        } while (0)
    switch(expr_tree->type) {
    case EXPR_NUMBER:
        MAKE_NUM_NODE_(result_tree, 0);
        return result_tree;
    case EXPR_VARIABLE:
        if (expr_tree->variable == diff_var) {
            MAKE_NUM_NODE_(result_tree, 1);
        } else {
            MAKE_NUM_NODE_(result_tree, 0);
        }
        return result_tree;
    case EXPR_OPERATION:
        {
            switch (expr_tree->operation) {
            case '+':
            case '-':
                MAKE_OP_NODE_(result_tree, expr_tree->operation);
                MAKE_DIFF_NODE_(result_tree->children[0], expr_tree->children[0]);
                MAKE_DIFF_NODE_(result_tree->children[1], expr_tree->children[1]);
                return result_tree;
            case '*':
                // d(L * R) = (dL * R) + (L * dR)
                MAKE_OP_NODE_  (result_tree, '+');

                MAKE_OP_NODE_  (result_tree->children[0], '*');
                MAKE_DIFF_NODE_(result_tree->children[0]->children[0], expr_tree->children[0]); // dL
                MAKE_COPY_NODE_(result_tree->children[0]->children[1], expr_tree->children[1]); // R

                MAKE_OP_NODE_  (result_tree->children[1], '*');
                MAKE_COPY_NODE_(result_tree->children[1]->children[0], expr_tree->children[0]); // L
                MAKE_DIFF_NODE_(result_tree->children[1]->children[1], expr_tree->children[1]); // dR

                return result_tree;
            case '/':
                // d(L / R) = ( (dL * R) - (L * dR) ) / (R * R)
                MAKE_OP_NODE_  (result_tree, '/');

                MAKE_OP_NODE_  (result_tree->children[0], '-');
                MAKE_OP_NODE_  (result_tree->children[0]->children[0], '*');
                MAKE_DIFF_NODE_(result_tree->children[0]->children[0]->children[0], expr_tree->children[0]); // dL
                MAKE_COPY_NODE_(result_tree->children[0]->children[0]->children[1], expr_tree->children[1]); // R
                MAKE_OP_NODE_  (result_tree->children[0]->children[1], '*');
                MAKE_COPY_NODE_(result_tree->children[0]->children[1]->children[0], expr_tree->children[0]); // L
                MAKE_DIFF_NODE_(result_tree->children[0]->children[1]->children[1], expr_tree->children[1]); // dR

                MAKE_OP_NODE_  (result_tree->children[1], '*');
                MAKE_COPY_NODE_(result_tree->children[1]->children[0], expr_tree->children[1]); // R
                MAKE_COPY_NODE_(result_tree->children[1]->children[1], expr_tree->children[1]); // R

                return result_tree;
            case '^':
                // d(L^R) = L^R * ( (dL) / L * R + (dR) * ln(L) )
                MAKE_OP_NODE_  (result_tree, '*');

                MAKE_OP_NODE_  (result_tree->children[0], '^');
                MAKE_COPY_NODE_(result_tree->children[0]->children[0], expr_tree->children[0]);
                MAKE_COPY_NODE_(result_tree->children[0]->children[1], expr_tree->children[1]);

                MAKE_OP_NODE_  (result_tree->children[1], '+');

                MAKE_OP_NODE_  (result_tree->children[1]->children[0], '*');
                MAKE_OP_NODE_  (result_tree->children[1]->children[0]->children[0], '/');
                MAKE_DIFF_NODE_(result_tree->children[1]->children[0]->children[0]->children[0], expr_tree->children[0]);
                MAKE_COPY_NODE_(result_tree->children[1]->children[0]->children[0]->children[1], expr_tree->children[0]);
                MAKE_COPY_NODE_(result_tree->children[1]->children[0]->children[1], expr_tree->children[1]);

                MAKE_OP_NODE_  (result_tree->children[1]->children[1], '*');
                MAKE_DIFF_NODE_(result_tree->children[1]->children[1]->children[0], expr_tree->children[1]);
                MAKE_OP_NODE_  (result_tree->children[1]->children[1]->children[1], 'l');
                MAKE_COPY_NODE_(result_tree->children[1]->children[1]->children[1]->children[1], expr_tree->children[0]);

                return result_tree;
            case 's':
                // d sin(R) = cos(R) * dR
                MAKE_OP_NODE_  (result_tree, '*');

                MAKE_OP_NODE_  (result_tree->children[0], 'c');
                MAKE_COPY_NODE_(result_tree->children[0]->children[1], expr_tree->children[1]);

                MAKE_DIFF_NODE_(result_tree->children[1], expr_tree->children[1]);

                return result_tree;
            case 'c':
                // d cos(R) = -sin(R) * dR
                MAKE_OP_NODE_  (result_tree, '-');
                MAKE_NUM_NODE_ (result_tree->children[0], 0);
                MAKE_OP_NODE_  (result_tree->children[1], '*');

                MAKE_OP_NODE_  (result_tree->children[1]->children[0], 's');
                MAKE_COPY_NODE_(result_tree->children[1]->children[0]->children[1], expr_tree->children[1]);

                MAKE_DIFF_NODE_(result_tree->children[1]->children[1], expr_tree->children[1]);

                return result_tree;
            case 'l':
                // d ln(R) = (dR) / R
                MAKE_OP_NODE_  (result_tree, '/');
                MAKE_DIFF_NODE_(result_tree->children[0], expr_tree->children[1]);
                MAKE_COPY_NODE_(result_tree->children[1], expr_tree->children[1]);

                return result_tree;
            default:
                destruct_expression(result_tree);
                return copy_expression(expr_tree);
            }
        default:
            destruct_expression(result_tree);
            return copy_expression(expr_tree);
        }
    }
    #undef MAKE_NUM_NODE_
    #undef MAKE_OP_NODE_
    #undef MAKE_COPY_NODE_
    #undef MAKE_DIFF_NODE_
}
