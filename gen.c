/* gen.c : code generator turns ast into VM bytecode. */
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

#include <stdio.h>
#include <ctype.h>

#include "ast.h"
#include "vm.h"
#include "gen.h"

struct codeinfo {
	vmcell *code;
	unsigned code_max;
};

static enum vmop vmop(enum ast_op op)
{
	switch (op) {
	case O_ADD: return IADD;
	case O_SUB: return ISUB;
	case O_MUL: return UMUL;
	case O_DIV: return UDIV;
	case O_ERR: ;
	}
	// TODO: an error occured...
	return HALT;
}

static void gen(vmcell v, struct codeinfo *info)
{
	*info->code++ = v;
	info->code_max--;
}

static void gen_2op(enum ast_op op, struct codeinfo *info)
{
	gen(vmop(op), info);
}

static void gen_num(long num, struct codeinfo *info)
{
	// TODO: support numbers of different sizes (64-bit, ...)
	gen(IPUSH, info);
	gen(num, info);
}

static void gen_var(const char *id, struct codeinfo *info)
{
	int i;

	/* TODO: use some allocation scheme */
	i = tolower(id[0]) - 'a';
	if (i < 0 || i >= 26)
		i = 0; // TODO: an error occured ...
	gen(IFETCH, info);
	gen(i, info);
}

static int c(ast_node node, struct codeinfo *info)
{
	switch (node->type) {
	case N_2OP:
		c(node->left, info);
		c(node->right, info);
		gen_2op(node->op, info);
		break;
	case N_NUM:
		gen_num(node->num, info);
		break;
	case N_VAR:
		gen_var(node->id, info);
		break;
	}
	return 0;
}

int compile(ast_node root, vmcell *code, unsigned code_max)
{
	struct codeinfo info = { code, code_max };
	int res;

	res = c(root, &info);
	gen(HALT, &info);
	printf("Code size = %d\n", code_max - info.code_max);
	/* TODO: it might be nice to return the size of the code object */
	return res;
}
