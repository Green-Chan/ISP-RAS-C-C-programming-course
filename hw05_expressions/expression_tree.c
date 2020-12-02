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
    for (size_t i = 0; i < sizeof(expr_tree->children) / sizeof(expr_tree->children[0]); i++) {
        destruct_expression(expr_tree->children[i]);
    }
    free(expr_tree);
}

static int read_expr_internal(const char *str, expression **expr_tree, const char **err_pos) {
    assert(str != NULL);
    assert(expr_tree != NULL);
    assert(err_pos != NULL);

    expr_skip_spaces(&str);
    if (str[0] != '(') {
        // No expression. And that is OK
        *expr_tree = NULL;
        *err_pos = str;
        return EXPR_OK;
    }
    // Skip '('
    str++;

    // Do not change expr_tree until you are sure, that this expression is ok.
    // As this function in recursive call follow this convention,
    // children of tmp_expr_tree are OK or NULL (because of calloc).
    // Thus we can call destruct_expression(tmp_expr_tree) without problems.
    expression *tmp_expr_tree = calloc(1, sizeof(expression));
    if (tmp_expr_tree == NULL) {
        return EXPR_OUT_OF_MEM;
    }

    // Read first argument
    int result = read_expr_internal(str, &tmp_expr_tree->children[0], err_pos);
    if (result != EXPR_OK) {
        // err_pos is already set correct by the last called read_expr_internal
        destruct_expression(tmp_expr_tree);
        return result;
    }

    str = *err_pos;
    expr_skip_spaces(&str);

    // Here are some local defines, that will be used in switch.
    // They should be undefined after it
    #ifdef ERROR_
        #error "That's a local define, it should not be defined"
    #endif
    #define ERROR_ *err_pos = str; \
                   destruct_expression(tmp_expr_tree); \
                   return EXPR_INVALID_STR;

    #ifdef SINCOS_
        #error "That's a local define, it should not be defined"
    #endif
    // Sine (cosine) should not have first argument
    #define SINCOS_ if (tmp_expr_tree->children[0] != NULL) { \
                        ERROR_ \
                    } \
                    tmp_expr_tree->type = EXPR_OPERATION; \
                    tmp_expr_tree->operation = str[0]; \
                    str += 3; // Skip operation string

    // If there was no argument, that's OK. It may be unary minus, sine, cosine, a number or a variable
    // read operation (spaces are already skipped)
    assert(!isspace(str[0]));
    switch(str[0]) {
    case '-':
        if (tmp_expr_tree->children[0] == NULL) {
            // Lets store unary minus as binary minus with zero first argument.
            // So put zero child and just go on, pretending that this zero was read
            tmp_expr_tree->children[0] = calloc(1, sizeof(expression));
            tmp_expr_tree->children[0]->type = EXPR_NUMBER;
        } // Fall through
    case '+':
    case '*':
    case '/':
    case '^':
        // For this operations first arguments should be
        if (tmp_expr_tree->children[0] == NULL) {
            ERROR_
        }
        tmp_expr_tree->type = EXPR_OPERATION;
        tmp_expr_tree->operation = str[0];
        // skip operation symbol
        str++;
        break;
    case 's':
        // That should be sine
        if (str[1] != 'i' || str[2] != 'n') {
            ERROR_
        }
        SINCOS_
        break;
    case 'c':
        // That should be cosine
        if (str[1] != 'o' || str[2] != 's') {
            ERROR_
        }
        SINCOS_
        break;
    default:
        // That is not operation, so it should be a number or a variable
        // and it shouldn't have first argument
        if (tmp_expr_tree->children[0] != NULL) {
            ERROR_
        } else {
            // Maybe it is a number?
            errno = 0;
            char *err_ptr;
            tmp_expr_tree->number = strtod(str, &err_ptr);
            if (err_ptr == str) {
                // That is not a number at all
                // Maybe it is a variable?
                if (isalpha(str[0])) {
                    tmp_expr_tree->type = EXPR_VARIABLE;
                    tmp_expr_tree->variable = str[0];
                    // Next not space symbol should be ')'
                    str++;
                    expr_skip_spaces(&str);
                    if (str[0] != ')') {
                        ERROR_
                    }
                    // Skip ')' and return
                    *err_pos = str + 1;
                    *expr_tree = tmp_expr_tree;
                    return EXPR_OK;
                } else {
                    ERROR_
                }
            } else if (errno == ERANGE) {
                // the number is too big and error is at err_ptr
                *err_pos = err_ptr;
                destruct_expression(tmp_expr_tree);
                return EXPR_INVALID_STR;
            } else {
                tmp_expr_tree->type = EXPR_NUMBER;
                // Number is already written in tmp_expr_tree->number
                // Next not space symbol should be ')'
                str = err_ptr;
                expr_skip_spaces(&str);
                if (str[0] != ')') {
                    ERROR_
                }
                // Skip ')' and return
                *err_pos = str + 1;
                *expr_tree = tmp_expr_tree;
                return EXPR_OK;
            }
        }
        // End of default case
        assert(!"should not be here");
    } // End of switch

    #undef ERROR_
    #undef SINCOS_

    // Read second argument
    result = read_expr_internal(str, &tmp_expr_tree->children[1], err_pos);
    if (result != EXPR_OK) {
        // err_pos is already set correct by the last called read_expr_internal
        destruct_expression(tmp_expr_tree);
        return result;
    }

    str = *err_pos;
    expr_skip_spaces(&str);
    if (str[0] != ')') {
        *err_pos = str;
        destruct_expression(tmp_expr_tree);
        return EXPR_INVALID_STR;
    }
    // Skip ')' and return
    *err_pos = str + 1;
    *expr_tree = tmp_expr_tree;
    return EXPR_OK;
}

int read_expression(const char *str, expression **expr_tree, const char **err_pos) {
    if (str == NULL || expr_tree == NULL) {
        return EXPR_INVALID_ARGS;
    }
    if (err_pos == NULL) {
        const char *tmp_err_pos;
        return read_expr_internal(str, expr_tree, &tmp_err_pos);
    } else {
        return read_expr_internal(str, expr_tree, err_pos);
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
        } else {
            printf("%c\n", expr_tree->operation);
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
        if (fprintf(fout, fmt, node_num, node_val) < 0) { \
            fclose(fout); \
            return 1; \
        }

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
        PRINT_(part_of_fmt ";\r\n", saved_node_number, node_val) \
        PRINT_("\t" part_of_fmt style_str "\r\n", saved_node_number, node_val) \
        for (size_t i = 0; i < sizeof(expr_tree->children) / sizeof(expr_tree->children[0]); i++) { \
            if (expr_tree->children[i] != NULL) { \
                PRINT_("\t" part_of_fmt "->", saved_node_number, node_val) \
                print_expr_tree_graph_internal(expr_tree->children[i], fout, node_number); \
            } \
        }

    switch(expr_tree->type) {
    case EXPR_NUMBER:
        PRINT_TREE_("\"node %I64d:\n%lf\"", "[style=\"filled\",fillcolor=\"yellow\"]", expr_tree->number)
        break;
    case EXPR_VARIABLE:
        PRINT_TREE_("\"node %I64d:\n%c\"", "[style=\"filled\",fillcolor=\"lightblue\"]", expr_tree->variable)
        break;
    case EXPR_OPERATION:
        if (expr_tree->operation == 's') {
            PRINT_TREE_("\"node %I64d:\n%s\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", "sin")
        } else if (expr_tree->operation == 'c') {
            PRINT_TREE_("\"node %I64d:\n%s\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", "cos")
        } else {
            PRINT_TREE_("\"node %I64d:\n%c\"", "[style=\"filled\",fillcolor=\"lightgreen\"]", expr_tree->operation)
        }
        break;
    default:
        PRINT_TREE_("\"node %I64d:\n%s\"", "", "??")
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
        if (check) { \
            return 1; \
        }

    #ifdef PRINT_CHILD_
        #error "That's a local define, it should not be defined"
    #endif
    #define PRINT_CHILD_(idx) \
        if (expr_tree->children[idx] == NULL) { \
            CHECK_RETURN_(fprintf(fout, "NOARG") < 0) \
        } else { \
            CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[idx], fout) != 0) \
        }

    #ifdef PRINT_ADD_SUB_CHILD_IN_BRACKETS_
        #error "That's a local define, it should not be defined"
    #endif
    #define PRINT_ADD_SUB_CHILD_IN_BRACKETS_(idx) \
        if (expr_tree->children[idx] == NULL) { \
            CHECK_RETURN_(fprintf(fout, "NOARG") < 0) \
        } else if (expr_tree->children[idx]->type == EXPR_OPERATION && \
                   (expr_tree->children[idx]->operation == '+' || \
                    expr_tree->children[idx]->operation == '-')){ \
            CHECK_RETURN_(fprintf(fout, "(") < 0) \
            CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[idx], fout) != 0) \
            CHECK_RETURN_(fprintf(fout, ")") < 0) \
        } else { \
            CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[idx], fout) != 0) \
        }


    switch(expr_tree->type) {
    case EXPR_NUMBER:
        if (expr_tree->number < 0) {
            CHECK_RETURN_(fprintf(fout, "(%lf)", expr_tree->number) < 0)
        } else {
            CHECK_RETURN_(fprintf(fout, "%lf", expr_tree->number) < 0)
        }
        return 0;
    case EXPR_VARIABLE:
        CHECK_RETURN_(fprintf(fout, "%c", expr_tree->variable) < 0)
        return 0;
    case EXPR_OPERATION:
        switch(expr_tree->operation) {
        case '+':
            PRINT_CHILD_(0)
            CHECK_RETURN_(fprintf(fout, " + ") < 0)
            PRINT_CHILD_(1)
            return 0;
        case '-':
            PRINT_CHILD_(0)
            CHECK_RETURN_(fprintf(fout, " - ") < 0)
            PRINT_ADD_SUB_CHILD_IN_BRACKETS_(1)
            return 0;
        case '*':
            PRINT_ADD_SUB_CHILD_IN_BRACKETS_(0)
            CHECK_RETURN_(fprintf(fout, " \\cdot ") < 0)
            PRINT_ADD_SUB_CHILD_IN_BRACKETS_(1)
            return 0;
        case '/':
            CHECK_RETURN_(fprintf(fout, "\\frac{") < 0)
            PRINT_CHILD_(0)
            CHECK_RETURN_(fprintf(fout, "}{") < 0)
            PRINT_CHILD_(1)
            CHECK_RETURN_(fprintf(fout, "}") < 0)
            return 0;
        case '^':
            if (expr_tree->children[0] == NULL) {
                CHECK_RETURN_(fprintf(fout, "NOARG") < 0)
            } else if (expr_tree->children[0]->type == EXPR_OPERATION) {
                #ifdef SIN_COS_POWER_PRINT_
                    #error "That's a local define, it should not be defined"
                #endif
                #define SIN_COS_POWER_PRINT_(sin_cos) \
                    CHECK_RETURN_(fprintf(fout, sin_cos "^{") < 0) \
                    PRINT_CHILD_(1) \
                    CHECK_RETURN_(fprintf(fout, "}(") < 0) \
                    if (expr_tree->children[0]->children[1] == NULL) { \
                        CHECK_RETURN_(fprintf(fout, "NOARG") < 0) \
                    } else { \
                        CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[0]->children[1], fout) != 0) \
                    } \
                    CHECK_RETURN_(fprintf(fout, ")") < 0) \

                if (expr_tree->children[0]->operation == 's') {
                    SIN_COS_POWER_PRINT_("sin")
                    return 0; // Everything is already printed
                } else if (expr_tree->children[0]->operation == 'c') {
                    SIN_COS_POWER_PRINT_("cos")
                    return 0; // Everything is already printed
                } else {
                    CHECK_RETURN_(fprintf(fout, "(") < 0)
                    CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[0], fout) != 0)
                    CHECK_RETURN_(fprintf(fout, ")") < 0)
                }
                #undef SIN_COS_POWER_PRINT_
            } else {
                CHECK_RETURN_(print_expr_formula_internal(expr_tree->children[0], fout) != 0)
            }
            // If we are here, only first argument is printed
            CHECK_RETURN_(fprintf(fout, "^{") < 0)
            PRINT_CHILD_(1)
            CHECK_RETURN_(fprintf(fout, "}") < 0)
            return 0;
        case 's':
            CHECK_RETURN_(fprintf(fout, "sin(") < 0)
            PRINT_CHILD_(1)
            CHECK_RETURN_(fprintf(fout, ")") < 0)
            return 0;
        case 'c':
            CHECK_RETURN_(fprintf(fout, "cos(") < 0)
            PRINT_CHILD_(1)
            CHECK_RETURN_(fprintf(fout, ")") < 0)
            return 0;
        default:
            CHECK_RETURN_(fprintf(fout, "UNKNOWN") < 0)
            return 0;
        }
        assert(!"should not be here");
    default:
        CHECK_RETURN_(fprintf(fout, "UNKNOWN") < 0)
        return 0;
    }
    assert(!"should not be here");
    #undef CHECK_RETURN_
    #undef PRINT_CHILD_
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
    size_t cmd_len = strlen("pdflatex \".tex\" -output-directory  >nul 2>nul") + 1 + n + n;
    char command[cmd_len];
    size_t real_cmd_len = sprintf(command, "pdflatex \"%s.tex\" ", name);
    size_t directory_end = 0;
    for (size_t i = 0; i < n; i++) {
        if (name[i] == '/') {
            directory_end = i;
        }
    }
    if (directory_end == 0) {
        real_cmd_len += sprintf(command + real_cmd_len, ">nul 2>nul");
    } else {
        buf[directory_end] = '\0';
        real_cmd_len += sprintf(command + real_cmd_len, "-output-directory %s >nul 2>nul", buf);
    }
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
        return 0;
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
            expr_tree->children[0] = simplify_expression(expr_tree->children[0], eps); \
            expr_tree->children[1] = simplify_expression(expr_tree->children[1], eps); \
            if (expr_tree->children[0] == NULL || expr_tree->children[1] == NULL) { \
                return expr_tree; \
            }
        #ifdef RETURN_CHILD_
            #error "That's a local define, it should not be defined"
        #endif
        #define RETURN_CHILD_(idx) \
            expression *return_tree = expr_tree->children[idx]; \
            expr_tree->children[idx] = NULL; \
            destruct_expression(expr_tree); \
            return return_tree;
        switch (expr_tree->operation) {
        case '+':
            SIMPLIFY_TWO_ARGS_
            if (expr_tree->children[0]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[0]->number, 0, eps)) {
                    RETURN_CHILD_(1)
                }
                if (expr_tree->children[1]->type == EXPR_NUMBER) {
                    expr_tree->children[1]->number += expr_tree->children[0]->number;
                    RETURN_CHILD_(1)
                }
            } else if (expr_tree->children[1]->type == EXPR_NUMBER && comp_eps(expr_tree->children[1]->number, 0, eps)) {
                RETURN_CHILD_(0)
            }
            return expr_tree;
        case '-':
            SIMPLIFY_TWO_ARGS_
            if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[1]->number, 0, eps)) {
                    RETURN_CHILD_(0)
                }
                if (expr_tree->children[0]->type == EXPR_NUMBER) {
                    expr_tree->children[0]->number -= expr_tree->children[1]->number;
                    RETURN_CHILD_(0)
                }
            }
            return expr_tree;
        case '*':
            SIMPLIFY_TWO_ARGS_
            if (expr_tree->children[0]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[0]->number, 0, eps)) {
                    RETURN_CHILD_(0)
                }
                if (comp_eps(expr_tree->children[0]->number, 1, eps)) {
                    RETURN_CHILD_(1)
                }
                if (expr_tree->children[1]->type == EXPR_NUMBER) {
                    expr_tree->children[1]->number *= expr_tree->children[0]->number;
                    RETURN_CHILD_(1)
                }
            } else if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[1]->number, 0, eps)) {
                    RETURN_CHILD_(1)
                }
                if (comp_eps(expr_tree->children[1]->number, 1, eps)) {
                    RETURN_CHILD_(0)
                }
            }
            return expr_tree;
        case '/':
            SIMPLIFY_TWO_ARGS_
            if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[1]->number, 1, eps)) {
                    RETURN_CHILD_(0)
                }
                if (expr_tree->children[0]->type == EXPR_NUMBER) {
                    expr_tree->children[0]->number /= expr_tree->children[1]->number;
                    RETURN_CHILD_(0)
                }
            } else if (expr_tree->children[0]->type == EXPR_NUMBER && comp_eps(expr_tree->children[0]->number, 0, eps)) {
                RETURN_CHILD_(0)
            }
            return expr_tree;
        case '^':
            SIMPLIFY_TWO_ARGS_
            if (expr_tree->children[0]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[0]->number, 0, eps)) {
                    RETURN_CHILD_(0)
                }
                if (comp_eps(expr_tree->children[0]->number, 1, eps)) {
                    RETURN_CHILD_(0)
                }
                if (expr_tree->children[1]->type == EXPR_NUMBER) {
                    expr_tree->children[0]->number = pow(expr_tree->children[0]->number, expr_tree->children[1]->number);
                    RETURN_CHILD_(0)
                }
            } else if (expr_tree->children[1]->type == EXPR_NUMBER) {
                if (comp_eps(expr_tree->children[0]->number, 0, eps)) {
                    expr_tree->children[1]->number = 1;
                    RETURN_CHILD_(1)
                }
                if (comp_eps(expr_tree->children[0]->number, 1, eps)) {
                    RETURN_CHILD_(0)
                }
            }
            return expr_tree;
        #ifdef SIN_COS_SIMP_
            #error "That's a local define, it should not be defined"
        #endif
        #define SIN_COS_SIMP_(sin_cos) \
            expr_tree->children[1] = simplify_expression(expr_tree->children[1], eps); \
            if (expr_tree->children[1] == NULL) { \
                return expr_tree; \
            } \
            if (expr_tree->children[1]->type == EXPR_NUMBER) { \
                expr_tree->children[1]->number = sin_cos (expr_tree->children[1]->number); \
                RETURN_CHILD_(1) \
            }
        case 's':
            SIN_COS_SIMP_(sin)
            return expr_tree;
        case 'c':
            SIN_COS_SIMP_(cos)
            return expr_tree;
        #undef SIN_COS_SIMP_
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
