/* gen.h */
#ifndef GEN_H
#define GEN_H
#include "vm.h"
int compile(ast_node root, vmcell *code, unsigned *code_max);
#endif
