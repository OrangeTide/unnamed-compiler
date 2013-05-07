/* vm.c : virtual machine executes a list of instructions. */
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

#include "trace.h"
#include "vm.h"


struct vmstate {
	vmcell pc;
	vmcell sp;
	vmcell stack[128];
	vmcell global[26];
	const vmcell *code;
	unsigned code_len;
};

static enum vmop vm_next(struct vmstate *vm)
{
	TRACE_FMT("pc:%04x\n", vm->pc);
	return vm->code[vm->pc++];
}

static vmcell vm_pcdata_next(struct vmstate *vm)
{
	return vm->code[vm->pc++];
}

static vmcell vm_global(struct vmstate *vm, unsigned i)
{
	return vm->global[i];
}

static void vm_global_set(struct vmstate *vm, unsigned i, vmcell v)
{
	vm->global[i] = v;
}

static void vm_push(struct vmstate *vm, vmcell v)
{
	TRACE_FMT("IPUSH %02x\n", v);
	vm->stack[vm->sp++] = v;
}

static vmcell vm_pop(struct vmstate *vm)
{
	TRACE_FMT("IPOP --- %02x\n", vm->stack[vm->sp - 1]);
	return vm->stack[--vm->sp];
}

struct vmstate *vm_new(const vmcell *code, unsigned code_len)
{
	struct vmstate *st;

	st = calloc(1, sizeof(*st));
	st->code = code; /* WARNING: code pointer must be preserved until vm_free() */
	st->code_len = code_len;
	return st;
}

void vm_free(struct vmstate *vm)
{
	free(vm);
}

int vm_run(struct vmstate *vm)
{
	TRACE;
	while (1) {
		if (vm->pc >= vm->code_len) {
			fprintf(stderr, "VM jumped out of bounds\n");
			return -1;
		}
		TRACE_FMT("\t\t%02X\n", vm->code[vm->pc]);
		switch (vm_next(vm)) {
		case HALT:
			// TODO: check for stack overflow
			printf("result = %d\n", vm->stack[vm->sp - 1]);
			return 0;
		case IFETCH:
			vm_push(vm, vm_global(vm, vm_pcdata_next(vm)));
			break;
		case ISTORE:
			vm_global_set(vm, vm_pcdata_next(vm), vm_pop(vm));
			break;
		case IPUSH:
			vm_push(vm, vm_pcdata_next(vm));
			break;
		case IPOP: /* TODO: rename this DROP */
			vm_pop(vm);
			break;
		case IADD:
			vm->sp--;
			vm->stack[vm->sp - 1] += vm->stack[vm->sp];
			break;
		case ISUB:
			vm->sp--;
			vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
			break;
		case UMUL:
			vm->sp--;
			vm->stack[vm->sp - 1] *= vm->stack[vm->sp];
			break;
		case UDIV: {
			vmcell d; // if this ever becomes signed division, handle negative overflow
			vm->sp--;
			d = vm->stack[vm->sp];
			if (d)
				vm->stack[vm->sp - 1] /= d;
			// TODO: else throw an exception
			break;
		}
		case ILT: /* Less than */
			vm->sp--;
			vm->stack[vm->sp - 1] =
				vm->stack[vm->sp - 1] < vm->stack[vm->sp];
			break;
		case JZ: { /* Jump if zero */
			vmcell ofs = vm_pcdata_next(vm) - 1;
			TRACE_FMT("JZ %+d\n", ofs);
			if (!vm_pop(vm)) {
				vm->pc += ofs;
				TRACE_FMT("\tjump to PC=%04x\n", vm->pc);
			}
			break;
		}
		case JNZ: { /* Jump if not zero */
			vmcell ofs = vm_pcdata_next(vm) - 1;

			TRACE_FMT("JNZ %+d\n", ofs);
			if (vm_pop(vm)) {
				vm->pc += ofs;
				TRACE_FMT("\tjump to PC=%04x\n", vm->pc);
			}
			break;
		}
		case JMP: { /* relative jump */
			vmcell ofs = vm_pcdata_next(vm) - 1;

			TRACE_FMT("JMP %+d\n", ofs);
			vm->pc += ofs;
			TRACE_FMT("\tjump to PC=%04x\n", vm->pc);
			break;
		}
		}
	}
}

void vm_dump(struct vmstate *vm)
{
	unsigned i;

	printf("code_len=%d\n", vm->code_len);
	for (i = 0; i < vm->code_len; i++) {
		printf("%04x %02x\n", i, vm->code[i]);
	}
}