/* ast.c - operations on the abstract syntax tree. */
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

static const char *opname(enum ast_op op)
{
	switch (op) {
	case O_ADD: return "+";
	case O_SUB: return "-";
	case O_MUL: return "*";
	case O_DIV: return "/";
	case O_ERR: ;
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
