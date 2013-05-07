#ifndef AST_H
#define AST_H
struct pstate;
typedef struct ast_node *ast_node;

enum ast_type {
	N_2OP, /* T_PLUS, T_MINUS, T_MUL, T_DIV */
	N_NUM, /* T_NUMBER */
	N_VAR, /* T_IDENTIFIER */
	N_COND, /* T_IF c T_THEN t T_ELSE f */
};

enum ast_op {
	O_ERR,
	O_ADD, O_SUB, O_MUL, O_DIV,
};

/* TODO: make this structure opaque. */
struct ast_node {
	enum ast_type type;
	ast_node left;
	union {
		struct {
			ast_node right; /* N_2OP */
			enum ast_op op;
		};
		long num; /* N_NUM */
		char *id; /* N_VAR */
		ast_node arg[2]; /* N_COND */
	};
	/* useful for error reporting during compile stage */
	unsigned line;
};

ast_node ast_node_new(struct pstate *st, enum ast_type type);
void ast_node_dump(const ast_node n);
void ast_node_free(ast_node n);
#endif
