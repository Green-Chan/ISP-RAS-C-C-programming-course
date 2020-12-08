#ifndef EXPRESSION_TREE_GUARD

typedef struct expression {
    char type;
    union {
       char operation;
       char variable;
       double number;
    };
    struct expression *children[2];
} expression;

//! Possible values of expression type
enum {
    EXPR_OPERATION = 'o',
    EXPR_VARIABLE = 'v',
    EXPR_NUMBER = 'n'
};

//! Return values of @c read_expression function
enum {
    EXPR_OK = 0,

    //! Invalid arguments were passed to @c read_expression
    EXPR_INVALID_ARGS,

    //! There is no valid expression in the beginning of the input string
    EXPR_INVALID_STR,

    //! Memory for all nodes of the tree could not be allocated
    EXPR_OUT_OF_MEM
};


//! Reads an expression from string and forms a tree to represent it.
//!
//! @param [in]  str        C-string (terminated by '\0'), the string with expression (as described at main page)
//! @param [out] expr_tree  Pointer to the tree (tree is always a pointer to the node structure)
//! @param [out] err_pos    The pointer to the first incorrect symbol, if function finishes with an error, or the pointer to the next symbol after expression if function succeeds
//!
//! @return 0 on success, error code otherwise
//!
//! @attention @c expr_tree will be rewrited on success, so don't forget to call @c destruct_expression first if there was another tree there
int read_expression(const char *str, expression **expr_tree, const char **err_pos);

//! Free memory, allocated for tree's nodes.
//!
//! @param [in] expr_tree  Pointer to the tree
void destruct_expression(expression *expr_tree);

//! Cope expression tree in new memory.
//!
//! @param [out] expr_tree  Pointer to the tree
//!
//! @return Pointer to the copy tree on success, 1 if memory for the new tree cannot be allocated
//!
//! @attention Always compare return value with 1
expression *copy_expression(expression *expr_tree);

//! Prints tree to the output and checks its correctness.
//!
//! @param [in] expr_tree  Pointer to the tree
//!
//! @note It prints tree even if it is not correct
void print_expr_tree_stdout(expression *expr_tree);

//! Prints tree in graph format. Create (or rewrite) <name>.dot and <name>.pdf files and opens PDF file.
//!
//! @param [in] expr_tree  Pointer to the tree
//! @param [in] name       C-string (terminated by '\0'), name for output files
//!
//! @return 0 on success, 1 if some problem with files occurs
//!
//! @note It prints tree even if it is not correct
//! @note @c name could contain a relative path
int print_expr_tree_graph(expression *expr_tree, const char *name);


//! Prints formula for expression. Create (or rewrite) <name>.tex , <name>.aux , <name>.log and <name>.pdf files and opens PDF file.
//!
//! @param [in] expr_tree  Pointer to the tree
//! @param [in] name       C-string (terminated by '\0'), name for output files.
//!
//! @return 0 on success, 1 if some problem with files occurs
//!
//! @note It prints tree even if it is not correct, but output is undefined. It also may look as it is correct
//! @note @c name could contain a relative path, but not absolute. Function works incorrect with absolute path
int print_expr_formula(expression *expr_tree, const char *name);

//! Simplify expression. Very simple simplifier: calculates subtrees which do not contain variables, reduces operations with neutral elements.
//!
//! @param [in] expr_tree  Pointer to the input expression tree
//! @param [in] eps        Epsilon which is used to compare numbers with neutral elements
//!
//! @return Pointer to the new root of expression tree
//!
//! @attention Changes input tree, can destruct some nodes
expression *simplify_expression(expression *expr_tree, double eps);

//! Create new expression tree, which is partial derivative of input expression.
//!
//! @param [in] expr_tree  Pointer to the input tree
//! @param [in] diff_var   A variable with respect to which expression is to be differentiated
//!
//! @return Pointer to the created expression tree on success, 1 if memory for the new tree cannot be allocated
//!
//! @attention Always compare return value with 1
expression *differentiate_expression(expression *expr_tree, char diff_var);

#define EXPRESSION_TREE_GUARD
#endif

