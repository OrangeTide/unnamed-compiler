#include <stdlib.h>

enum vmop {
	HALT, IFETCH, ISTORE, IPUSH, IPOP, IADD, ISUB, ILT, JZ, JNZ, JMP,
};

typedef unsigned vmcell;

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
	vm->stack[vm->sp++] = v;
}

static vmcell vm_pop(struct vmstate *vm)
{
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
	while (1) {
		if (vm->pc >= vm->code_len)
			return -1;
		switch (vm_next(vm)) {
		case HALT:
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
			vm->stack[vm->sp - 1] += vm->stack[vm->sp];
			break;
		case ILT:
			vm->sp--;
			vm->stack[vm->sp - 1] =	vm->stack[vm->sp - 1] < vm->stack[vm->sp];
			break;
		case JZ: {
			vmcell ofs = vm_pcdata_next(vm);

			if (!vm_pop(vm))
				vm->pc = ofs;
			}
			break;
		case JNZ: {
			vmcell ofs = vm_pcdata_next(vm);

			if (vm_pop(vm))
				vm->pc += ofs;
			}
			break;
		case JMP: /* relative jump */
			vm->pc += vm_pcdata_next(vm);
			break;
		}
	}
}