/* parse.c - parser turns tokens into ast(abstract syntax tree). */
/*
 * Copyright (c) 2013 Jon Mayo <jon@rm-f.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Syntax:
 *
 * Expr ::= Term { '+' Term }
 * Term ::= Factor { '*' Factor }
 * Factor ::= identifier | number | "(" Expr ")"
 * number ::= [0-9]+
 * identifier ::= [A-Za-z_][A-Za-z0-9_]*
 *
 * Future:
 * assignStmt := lvalue = expr
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "tok.h"
#include "trace.h"

ast_node expr(struct pstate *st); /* forward */

static enum ast_op op(enum token t)
{
	switch (t) {
	case T_PLUS: return O_ADD;
	case T_MINUS: return O_SUB;
	case T_MUL: return O_MUL;
	case T_DIV: return O_DIV;
	default:
		return O_ERR;
	}
}

ast_node number(struct pstate *st)
{
	ast_node n;

	TRACE;
	n = ast_node_new(st, N_NUM);
	n->num = num_buf(st);
	tok_next(st);
	return n;
}

ast_node identifier(struct pstate *st)
{
	ast_node n;

	TRACE;
	n = ast_node_new(st, N_VAR);
	n->id = strdup(id_buf(st));
	tok_next(st);
	return n;
}

/* ExprParen ::= "(" Expr ")" */
ast_node expr_paren(struct pstate *st)
{
	ast_node n;

	TRACE;
	tok_next(st);
	n = expr(st);
	if (!n)
		return NULL;
	if (tok_cur(st) != T_RIGHT_PAREN) {
		error(st, "missing parenthesis");
		return NULL;
	}
	tok_next(st);
	return n;
}

/* Factor ::= identifier | number | "(" Expr ")"
 */
ast_node factor(struct pstate *st)
{
	TRACE;
	if (tok_cur(st) == T_EOF) {
		fprintf(stderr, "<EOF>\n");
		return NULL; /* ast_node_new(st, T_EOF); */
	}
	if (tok_cur(st) == T_IDENTIFIER) {
		return identifier(st);
	} else if (tok_cur(st) == T_NUMBER) {
		return number(st);
	} else if (tok_cur(st) == T_LEFT_PAREN) { /* "(" Expr ")" */
		return expr_paren(st);
	}

	error(st, "missing identifier or number");
	return NULL;
}

/* Term ::= Factor { "*" Factor }
 */
ast_node term(struct pstate *st)
{
	ast_node left;

	TRACE;
	left = factor(st);
	if (!left)
		return NULL;
	while (tok_cur(st) == T_MUL || tok_cur(st) == T_DIV) {
		ast_node new = ast_node_new(st, N_2OP);

		new->op = op(tok_cur(st));
		tok_next(st);
		new->left = left;
		new->right = factor(st);
		left = new; /* recurse left */
	}

	return left;
}

/* Expr ::= Term { "+" Term }
 */
ast_node expr(struct pstate *st)
{
	ast_node left;

	TRACE;
	/* TODO: check for T_MINUS to find unary +/- */
	left = term(st);
	if (!left)
		return NULL;
	while (tok_cur(st) == T_PLUS || tok_cur(st) == T_MINUS) {
		ast_node new = ast_node_new(st, N_2OP);

		new->op = op(tok_cur(st));
		tok_next(st);
		new->left = left;
		new->right = term(st);
		left = new; /* recurse left */
	}

	return left;
}

ast_node parse(void)
{
	struct pstate *st;
	ast_node root;

	st = pstate_new();
	root = expr(st);
	discard_whitespace(st);
	if (tok_cur(st) != T_EOF)
		error(st, "trailing garbage");
	TRACE_FMT("DONE!\n");
	if (last_error(st)) {
		pstate_free(st);
		return NULL;
	}

	pstate_free(st);
	return root;
}

/*** MAIN ***/
#include "gen.h"

#define CODE_MAX 2048 /* maximum compiled size*/

int main()
{
	vmcell code[CODE_MAX];
	struct vmstate *vm;

	printf("Parsing...\n");
	ast_node root = parse();
	ast_node_dump(root);
	printf("\n");

	printf("Compiling...\n");
	compile(root, code, sizeof(code));
	/* TODO: ast_node_free(root); */

	printf("Running...\n");
	vm = vm_new(code, sizeof(code));
	vm_run(vm);
	vm_free(vm);
	printf("Done!\n");
	return 0;
}
