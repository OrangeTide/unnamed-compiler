#ifndef VM_H
#define VM_H
typedef unsigned vmcell;

enum vmop {
	HALT, IFETCH, ISTORE, IPUSH, IPOP,
	IADD, ISUB, UMUL, UDIV,
	ILT,
	JZ, JNZ, JMP,

};

struct vmstate;

struct vmstate *vm_new(const vmcell *code, unsigned code_len);
void vm_free(struct vmstate *vm);
int vm_run(struct vmstate *vm);
void vm_dump(struct vmstate *vm);
#endif
