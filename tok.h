#ifndef TOK_H
#define TOK_H
struct pstate;

enum token {
	T_EOF,
	T_IDENTIFIER,
	T_NUMBER,
	T_PLUS, T_MINUS, T_MUL, T_DIV,
	T_LEFT_PAREN, T_RIGHT_PAREN,
};

void error(struct pstate *st, const char *reason);
void ch_next(struct pstate *st);
int ch_cur(struct pstate *st);
int last_error(struct pstate *st);
int line_cur(struct pstate *st);
long num_buf(struct pstate *st);
const char *id_buf(struct pstate *st);
int eat(struct pstate *st, char c);
int require(struct pstate *st, char *str);
void discard_whitespace(struct pstate *st);
void tok_next(struct pstate *st);
int tok_cur(struct pstate *st);
struct pstate *pstate_new(void);
void pstate_free(struct pstate *st);
#endif
