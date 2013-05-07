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
#include "trace.h"

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

/* current posisition in the generated object file */
static vmcell *here(struct codeinfo *info)
{
	return info->code;
}

/* reserve a cell, returning it's offset */
static vmcell *hole(struct codeinfo *info)
{
	vmcell *pos = here(info);

	gen(0xdeadbeef, info); /* store a dummy value */
	return pos;
}

/* patch a memory location at src with the offset to dst. */
static void fix(vmcell *src, const vmcell *dst)
{
	*src = dst - src;
	TRACE_FMT("fix %04x\n", *src);
}

static int c(ast_node node, struct codeinfo *info)
{
	switch (node->type) {
	case N_2OP:
		c(node->left, info);
		c(node->right, info);
		gen_2op(node->op, info);
		return 1;
	case N_NUM:
		gen_num(node->num, info);
		return 1;
	case N_VAR:
		gen_var(node->id, info);
		return 1;
	case N_COND: {
		vmcell *patch1, *patch2;

		/* TODO: support conditions missing an else ... */

		c(node->left, info); /* condition */
		gen(JZ, info); patch1 = hole(info); /* calculate JZ's destination later... */
		c(node->arg[0], info); /* true condition */
		gen(JMP, info); patch2 = hole(info); /* calculate JMP's destination later... */
		fix(patch1, here(info)); /* destination for JZ */
		c(node->arg[1], info); /* false condition */
		fix(patch2, here(info)); /* destination for JMP */
		TRACE_FMT("patch1=%04x patch2=%04x\n", *patch1, *patch2);
		return 1;
	}
	}
	/* TODO: report compile error on the first failure */
	return 0;
}

int compile(ast_node root, vmcell *code, unsigned *code_max)
{
	struct codeinfo info = { code, *code_max };
	int res;

	res = c(root, &info);
	gen(HALT, &info);
	*code_max = *code_max - info.code_max;
	printf("Code size = %d\n", *code_max);
	return res;
}
