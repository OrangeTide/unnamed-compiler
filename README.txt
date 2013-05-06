Language
========

Syntax:

Expr ::= Term { '+' Term }
Term ::= Factor { '*' Factor }
ExprParen ::= "(" Expr ")"
Factor ::= identifier | number | ExprParen
number ::= [0-9]+
identifier ::= [A-Za-z_][A-Za-z0-9_]*

Current Limitations
===================

Because there is no way to assign values to variables(identifiers), they are
always 0.

Identifiers only matter for the first letter, the remaining letters are ignored.
And only 'a' to 'z' are supported, for 26 total global values.

Files
=====

ast.c : operations on the abstract syntax tree.
gen.c : code generator turns ast into VM bytecode.
lang.c : the main function for the language.
parse.c : parser turns tokens into ast(abstract syntax tree).
tok.c : lexer turns input into tokens.
vm.c : virtual machine executes a list of instructions.

TODO
====

1. Add the following syntax:

	assignStmt := identifier '=' ExprParen

2. Allocate identifiers and assign them slots in the global variables table.

3. Free data structures and don't leak memory.



