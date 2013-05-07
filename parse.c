/* parse.c : parser turns tokens into ast(abstract syntax tree). */
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
 * ExprTerm ::= Term { '+' Term }
 * Term ::= Factor { '*' Factor }
 * Expr ::= IfExpr | ExprTerm
 * ExprParen ::= "(" Expr ")"
 * Factor ::= identifier | number | ExprParen
 * number ::= [0-9]+
 * identifier ::= [A-Za-z_][A-Za-z0-9_]*
 * IfExpr ::= "if" ExprParen "then" Expr "else" Expr
 *
 * Future:
 * AssignStmt ::= identifier '=' ExprParen
 * ExprList ::= Expr*
 * BlockExpr ::= "{" ExprList "}"
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "tok.h"
#include "trace.h"
#include "parse.h"

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

/* number ::= [0-9]+
 */
ast_node number(struct pstate *st)
{
	ast_node n;

	TRACE;
	n = ast_node_new(st, N_NUM);
	n->num = num_buf(st);
	tok_next(st);
	return n;
}

/* identifier ::= [A-Za-z_][A-Za-z0-9_]*
 */
ast_node identifier(struct pstate *st)
{
	ast_node n;

	TRACE;
	n = ast_node_new(st, N_VAR);
	n->id = strdup(id_buf(st));
	tok_next(st);
	return n;
}

/* ExprParen ::= "(" Expr ")"
 */
ast_node paren_expr(struct pstate *st)
{
	ast_node n;

	TRACE;
	tok_next(st);
	n = expr(st);
	if (!n)
		return NULL;
	TRACE;
	if (tok_cur(st) != T_RIGHT_PAREN) {
		TRACE;
		TRACE_FMT("ERROR:tok=%d\n", tok_cur(st));
		error(st, "missing parentheses");
		ast_node_free(n);
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
	if (tok_cur(st) == T_IDENTIFIER) {
		return identifier(st);
	} else if (tok_cur(st) == T_NUMBER) {
		return number(st);
	} else if (tok_cur(st) == T_LEFT_PAREN) { /* "(" Expr ")" */
		return paren_expr(st);
	}
	return NULL; /* ignore anything else as not a match */
}

ast_node factor_required(struct pstate *st)
{
	ast_node n = factor(st);

	if (!n)
		error(st, "missing identifier or number");
	return n;
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
		new->right = factor_required(st);
		if (!new->right) {
			ast_node_free(new);
			return NULL;
		}
		left = new; /* recurse left */
	}

	return left;
}

ast_node expr_term(struct pstate *st)
{

	ast_node left;

	TRACE;
	/* TODO: check for T_MINUS to find unary +/- */
	left = term(st);
	TRACE_FMT("%s():%d:tok=%d\n", __func__, __LINE__, tok_cur(st));
	if (!left)
		return NULL;
	while (tok_cur(st) == T_PLUS || tok_cur(st) == T_MINUS) {
		ast_node new = ast_node_new(st, N_2OP);

		TRACE;
		new->op = op(tok_cur(st));
		tok_next(st);
		new->left = left;
		new->right = term(st);
		if (!new->right) {
			error(st, "missing factor");
			ast_node_free(new);
			return NULL;
		}
		left = new; /* recurse left */
	}

	return left;
}

/* IfExpr ::= "if" ParenExpr "then" Expr "else" Expr
 */
ast_node if_expr(struct pstate *st)
{
	ast_node n;


	TRACE;
	if (tok_cur(st) != T_IF) {
		error(st, "missing 'if'");
		return NULL;
	}
	tok_next(st);

	n = ast_node_new(st, N_COND);
	n->left = paren_expr(st); /* condition */
	TRACE;
	if (tok_cur(st) != T_THEN) {
		error(st, "missing 'then'");
		ast_node_free(n);
		return NULL;
	}

	tok_next(st);
	n->arg[0] = expr(st); /* then */

	if (tok_cur(st) == T_ELSE) {
		tok_next(st);
		n->arg[1] = expr(st); /* else */
	} else {
		n->arg[1] = NULL;
	}
	TRACE;
	return n;
}

/* Expr ::= Term { "+" Term }
 */
ast_node expr(struct pstate *st)
{
	TRACE;
	if (tok_cur(st) == T_IF) {
		TRACE;
		return if_expr(st);
	} else {
		TRACE;
		return expr_term(st);
	}
}

ast_node parse(void)
{
	struct pstate *st;
	ast_node root;

	st = pstate_new();
	root = expr(st);
	discard_whitespace(st);
	TRACE_FMT("final token=%d\n", tok_cur(st));
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
