/* ast.c - operations on the abstract syntax tree */
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "tok.h"

ast_node ast_node_new(struct pstate *st, enum ast_type type)
{
	ast_node n;
	n = calloc(1, sizeof(*n));
	n->type = type;
	n->op = ~0;
	n->line = line_cur(st);
	return n;
}

static const char *opname(int op)
{
	switch (op) {
	case T_PLUS: return "+";
	case T_MINUS: return "-";
	case T_MUL: return "*";
	case T_DIV: return "/";
	}
	return "UNKNOWN";
}

void ast_node_dump(const ast_node n)
{
	if (!n)
		return;

	switch (n->type) {
	case N_2OP:
		printf(" (%s", opname(n->op));
		ast_node_dump(n->left);
		ast_node_dump(n->right);
		printf(")");
		return;
	case N_NUM:
		printf(" %ld", n->num);
		return;
	case N_VAR:
		printf(" %s", n->id);
		return;
	}

	printf("ERROR\n");
}
