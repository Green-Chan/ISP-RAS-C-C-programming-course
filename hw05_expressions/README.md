# Expression tree

## Description

Here is a set of functions to work with expressions. Correct expression could be a double-precision floating-point number, a variable (one letter from the English alphabet)
or an operation on arguments, which are correct expressions.
Here are the list of supported operations:
| Operation | Meaning     | Usage           |
|-----------|-------------|-----------------|
| +         | plus        | ((5) + (3))     |
| -         | minus       | ((7)- (a))      |
| -         | unary minus | (-(Z))          |
| *         | multiply    | ((7) *(9.234))  |
| /         | divide      | ((2.3)/(x))     |
| ^         | power       | ((2)  ^ (10) )  |
| sin       | sine        | (sin(5.0))      |
| cos       | cosine      | (  cos (1))     |
| ln        | natural logarithm | (ln(4))   |

In usage you can see correct examples of the input for read_expression. The whole expression and each sub-expression (argument of the operation)
should be in round brackets.

The expression is stored in the binary tree. Pay attention, that unary minus is not stored as operation with one argument, but as binary minus
with zero first argument, so printed formula (output of print_expr_formula function) sometimes looks not as it could be expected.

There are two functions to print expression tree as a graph: print_expr_tree_stdout and print_expr_tree_graph .

You can also copy expression, simplify it and differentiate it with respect to some variable.

## Getting Started

### Dependencies

* Windows

* MinGW

* LaTeX (add path to the "pdflatex" command to the PATH)

* Graphviz (add path to the "dot" command to the PATH)

### Installing

* download directories hw05_expressions from https://github.com/Green-Chan/ISP_RAS_programming_course

### Building

* Open CMD

* Get to the hw05_expressions directory
```
> cd path\to\the\directory\hw05_expressions
```
> **Note:** If you need to change the drive from C: to D:, type "D:" (without quotes).

* Run mingw32-make
```
> mingw32-make
```
> **Note:** you may need to learn how to add MinGW\bin to the PATH (the Environment Settings section in http://www.mingw.org/wiki/Getting_Started/).

### Running tests

* Run mingw32-make with argument test
```
> mingw32-make test
```
> **Note:** mingw32-make test execute run_tests.exe redirecting input

Be careful, this will create and open a lot of PDF files.

You can also use run_tests.exe with your own expressions:
Run program
```
> run_tests.exe
```
Then write your expression in format in which it can be read by read_expression. Then press Enter.

Expression, its copy, simplified expression, derivative of simplified expression, simplified derivative of simplified expression will be printed in three formats:
tree in stdout, tree in PDF, formula in PDF. And it will open PDF files. Files are stored in directory hw05_expressions\output_files .

Then you can write another expression and get the same for it. To end the program press Ctrl+C.

Pay attention that each run of program rewrite output files of previous run, so if you need some of that files, copy them to other directory.

## Debugging

To debug the program using GDB:
* Rebuild the program with DEBUG=1
```
> mingw32-make clean
> mingw32-make DEBUG=1
```
* Run program using GDB
```
> gdb run_tests.exe
```

## Documentation

You can find documentation, generated by Doxygen in hw05_expressions\documentation directory. If you have Doxygen and know how to use it,
you can generate documentation yourself using Doxyfile in the same directory hw05_expressions\documentation).

## Authors

Litskevich Karina

Contact me on telegram: [@karkarik](https://t.me/karkarik)