#ifndef AST_H
#define AST_H
struct pstate;
typedef struct ast_node *ast_node;

enum ast_type {
	N_2OP, /* T_PLUS, T_MINUS, T_MUL, T_DIV */
	N_NUM, /* T_NUMBER */
	N_VAR, /* T_IDENTIFIER */
};

/* TODO: make this structure opaque. */
struct ast_node {
	enum ast_type type;
	ast_node left;
	union {
		struct {
			ast_node right; /* N_2OP */
			int op;
		};
		long num; /* N_NUM */
		char *id; /* N_VAR */
	};
	/* useful for error reporting during compile stage */
	unsigned line;
};

ast_node ast_node_new(struct pstate *st, enum ast_type type);
void ast_node_dump(const ast_node n);
#endif
