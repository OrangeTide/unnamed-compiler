/* lang.c : the main function for the language. */
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

#include "ast.h"
#include "parse.h"
#include "gen.h"

#define CODE_MAX 2048 /* maximum compiled size */

int main()
{
	vmcell code[CODE_MAX];
	unsigned code_len = CODE_MAX;
	ast_node root;

	printf("Parsing...\n");
	root = parse();
	if (!root) {
		fprintf(stderr, "PARSE ERROR!\n");
		return 1;
	}
	ast_node_dump(root);
	printf("\n");

	printf("Compiling...\n");

	if (!compile(root, code, &code_len)) {
		fprintf(stderr, "COMPILE ERROR!\n");
		ast_node_free(root);
		return 1;
	}

	struct vmstate *vm;
	ast_node_free(root);
	printf("Running...\n");
	vm = vm_new(code, code_len);
#ifndef NDEBUG
	vm_dump(vm);
#endif
	vm_run(vm);
	vm_free(vm);
	printf("Done!\n");

	return 0;
}
