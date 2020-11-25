#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "expression_tree.h"

void expr_skip_spaces(const char **str) {
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

int read_expr_internal(const char *str, expression **expr_tree, const char **err_pos) {
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
        destruct_expression(tmp_expr_tree);
        return result;
    }

    str = *err_pos;
    expr_skip_spaces(&str);
    // If there was no argument, that's OK
    if (tmp_expr_tree->children[0] == NULL) {
        // It may be number or unary minus
        // Note: in this version negative number should be written (-(<num>))
        if (str[0] == '-') {
            // Lets store unary minus as binary minus with zero first argument.
            // So put zero child and just go on, pretending that this zero was read
            tmp_expr_tree->children[0] = calloc(1, sizeof(expression));
        } else {
            errno = 0;
            char *err_ptr;
            tmp_expr_tree->number = strtod(str, &err_ptr);
            if (err_ptr == str || errno == ERANGE) {
                // there is an error at err_ptr
                *err_pos = err_ptr;
                destruct_expression(tmp_expr_tree);
                return EXPR_INVALID_STR;
            } else {
                // Next not space symbol should be ')'
                str = err_ptr;
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
        }
    }
    // If there is an argument, read operation (spaces are already skipped)
    assert(!isspace(str[0]));
    switch(str[0]) {
    case '+':
    case '-':
    case '*':
    case '/':
        tmp_expr_tree->operation = str[0];
        str++;
        break;
    default:
        *err_pos = str;
        destruct_expression(tmp_expr_tree);
        return EXPR_INVALID_STR;
    }

    // Read second argument
    result = read_expr_internal(str, &tmp_expr_tree->children[1], err_pos);
    if (result != EXPR_OK) {
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
    static size_t h = 0;
    if (expr_tree == NULL) {
        return;
    }
    if (expr_tree->children[0] == NULL) {
        for (size_t i = 0; i < h; i++) {
            printf("  ");
        }
        printf("%lf\n", expr_tree->number);
        return;
    }
    h++;
    print_expr_tree_stdout(expr_tree->children[0]);
    h--;
    for (size_t i = 0; i < h; i++) {
        printf("  ");
    }
    printf("%c\n", expr_tree->operation);
    h++;
    print_expr_tree_stdout(expr_tree->children[1]);
    h--;
}

int print_expr_tree_graph_internal(expression *expr_tree, FILE *fout, size_t *node_number) {
    assert(expr_tree != NULL);
    assert(expr_tree->children[0] != NULL);
    assert(expr_tree->children[1] != NULL);
    size_t saved_node_number = *node_number;
    if (fprintf(fout, "\t\"node %I64d:\n%c\"[style=\"filled\",fillcolor=\"lightgreen\"]\r\n", saved_node_number, expr_tree->operation) < 0) {
        fclose(fout);
        return 1;
    }
    for (size_t i = 0; i < 2; i++) {
        if (expr_tree->children[i]->children[i] == NULL) {
            *node_number += 1;
            if (fprintf(fout, "\t\"node %I64d:\n%lf\"[style=\"filled\",fillcolor=\"yellow\"]\t\"node %I64d:\n%c\"->\"node %I64d:\n%lf\";\r\n",
                        *node_number, expr_tree->children[i]->number, saved_node_number,
                        expr_tree->operation, *node_number, expr_tree->children[i]->number) < 0) {
                fclose(fout);
                return 1;
            }
        } else {
            *node_number += 1;
            if (fprintf(fout, "\t\"node %I64d:\n%c\"->\"node %I64d:\n%c\";\r\n", saved_node_number,
                        expr_tree->operation, *node_number, expr_tree->children[i]->operation) < 0) {
                fclose(fout);
                return 1;
            }
            if (print_expr_tree_graph_internal(expr_tree->children[i], fout, node_number) != 0) {
                return 1;
            }
        }
    }
    return 0;
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
    } else if (expr_tree->children[0] == NULL) {
        if (fprintf(file_out, "\t\"node 0:\n%lf\"[style=\"filled\",fillcolor=\"yellow\"];\r\n", expr_tree->number) < 0) {
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

int print_expr_formula_internal(expression *expr_tree, FILE *fout) {
    assert(expr_tree != NULL);
    if (expr_tree->children[0] == NULL) {
        if (fprintf(fout, "%lf", expr_tree->number) < 0) {
            fclose(fout);
            return 1;
        }
    } else {
        assert(expr_tree->children[1] != NULL);
        switch(expr_tree->operation) {
        case '+':
            if (print_expr_formula_internal(expr_tree->children[0], fout) != 0) {
                fclose(fout);
                return 1;
            }
            if (fprintf(fout, " + ") < 0) {
                fclose(fout);
                return 1;
            }
            if (print_expr_formula_internal(expr_tree->children[1], fout) != 0) {
                fclose(fout);
                return 1;
            }
            break;
        case '-':
            if (print_expr_formula_internal(expr_tree->children[0], fout) != 0) {
                fclose(fout);
                return 1;
            }
            if (fprintf(fout, " - ") < 0) {
                fclose(fout);
                return 1;
            }
            if (expr_tree->children[1]->children[0] != NULL &&
                    (expr_tree->children[1]->operation == '+'
                     || expr_tree->children[1]->operation == '-')) {
                if (fprintf(fout, "(") < 0) {
                    fclose(fout);
                    return 1;
                }
                if (print_expr_formula_internal(expr_tree->children[1], fout) != 0) {
                    fclose(fout);
                    return 1;
                }
                if (fprintf(fout, ")") < 0) {
                    fclose(fout);
                    return 1;
                }
            } else {
                if (print_expr_formula_internal(expr_tree->children[1], fout) != 0) {
                    fclose(fout);
                    return 1;
                }
            }
            break;
        case '*':
            if (expr_tree->children[0]->children[0] != NULL &&
                    (expr_tree->children[0]->operation == '+'
                     || expr_tree->children[0]->operation == '-')) {
                if (fprintf(fout, "(") < 0) {
                    fclose(fout);
                    return 1;
                }
                if (print_expr_formula_internal(expr_tree->children[0], fout) != 0) {
                    fclose(fout);
                    return 1;
                }
                if (fprintf(fout, ")") < 0) {
                    fclose(fout);
                    return 1;
                }
            } else {
                if (print_expr_formula_internal(expr_tree->children[0], fout) != 0) {
                    fclose(fout);
                    return 1;
                }
            }

            if (fprintf(fout, " \\cdot ") < 0) {
                fclose(fout);
                return 1;
            }

            if (expr_tree->children[1]->children[0] != NULL &&
                    (expr_tree->children[1]->operation == '+'
                     || expr_tree->children[1]->operation == '-')) {
                if (fprintf(fout, "(") < 0) {
                    fclose(fout);
                    return 1;
                }
                if (print_expr_formula_internal(expr_tree->children[1], fout) != 0) {
                    fclose(fout);
                    return 1;
                }
                if (fprintf(fout, ")") < 0) {
                    fclose(fout);
                    return 1;
                }
            } else {
                if (print_expr_formula_internal(expr_tree->children[1], fout) != 0) {
                    fclose(fout);
                    return 1;
                }
            }
            break;
        case '/':
            if (fprintf(fout, "\\frac{") < 0) {
                fclose(fout);
                return 1;
            }
            if (print_expr_formula_internal(expr_tree->children[0], fout) != 0) {
                fclose(fout);
                return 1;
            }
            if (fprintf(fout, "}{") < 0) {
                fclose(fout);
                return 1;
            }
            if (print_expr_formula_internal(expr_tree->children[1], fout) != 0) {
                fclose(fout);
                return 1;
            }
            if (fprintf(fout, "}") < 0) {
                fclose(fout);
                return 1;
            }
            break;
        default:
            assert(!"impossible case");
        }
    }
    return 0;
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
    size_t cmd_len = strlen("pdflatex \".tex\"") + 1 + n;
    char command[cmd_len];
    size_t real_cmd_len = sprintf(command, "pdflatex \"%s.tex\"", name);
    assert(real_cmd_len + 1 <= cmd_len);
    command[real_cmd_len] = '\0';
    system(command);

    real_cmd_len = sprintf(command, "start \"\" \"%s.pdf\"", name);
    assert(real_cmd_len + 1 <= cmd_len);
    command[real_cmd_len] = '\0';
    system(command);
    return 0;
}
