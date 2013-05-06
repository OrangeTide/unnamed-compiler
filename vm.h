#ifndef VM_H
#define VM_H
typedef unsigned vmcell;

enum vmop {
	HALT, IFETCH, ISTORE, IPUSH, IPOP, IADD, ISUB, ILT, JZ, JNZ, JMP,
	UMUL, UDIV,
};

struct vmstate;

struct vmstate *vm_new(const vmcell *code, unsigned code_len);
void vm_free(struct vmstate *vm);
int vm_run(struct vmstate *vm);

#endif
