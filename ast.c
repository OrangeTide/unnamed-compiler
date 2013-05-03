/*
 *
 * Syntax:
 *
 * Expr ::= Term { '+' Term }
 * Term ::= Factor { '*' Factor }
 * Factor ::= identifier | number | "(" Expr ")"
 * number ::= [0-9]+
 * identifier ::= [A-Za-z_][A-Za-z0-9_]*
 *
 * Future:
 * assignStmt := lvalue = expr
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*** PARSER ***/

enum token {
	T_EOF,
	T_IDENTIFIER,
	T_NUMBER,
	T_PLUS, T_MINUS, T_MUL, T_DIV,
	T_LEFT_PAREN, T_RIGHT_PAREN,
};

/* parser state */
struct pstate {
	int ch;
	int error;
	enum token tok;
	int line;
	int offset;
	long num_buf;
	char id_buf[64];
};

void error(struct pstate *st, const char *reason)
{
	st->error = 1;
	st->tok = T_EOF;
	fprintf(stderr, "line=%d,ofs=%d:%s\n", st->line, st->offset, reason);
}

void ch_next(struct pstate *st)
{
	if (st->ch == EOF)
		return;
	st->ch = getchar();
	st->offset++;
	if (st->ch == '\n') {
		st->line++;
		st->offset = 0;
	}
}

int ch_cur(struct pstate *st)
{
	return st->error ? EOF : st->ch;
}

int eat(struct pstate *st, char c)
{
	int r;

	r = st->ch == c;
	if (r)
		ch_next(st);
	return r;
}

int require(struct pstate *st, char *str)
{
	while (*str && eat(st, *str))
			str++;
	if (*str != 0)  /* else we could not consume the entire string, which is bad and we must give up! */
		error(st, "keyword expected");
	return !st->error;
}

void discard_whitespace(struct pstate *st)
{
	fprintf(stderr, "TRACE:%s()\n", __func__);
	while (isspace(ch_cur(st)))
		ch_next(st);
}

void tok_next(struct pstate *st)
{
	char ch;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	discard_whitespace(st); /* TODO: is this correct?? */
	ch = ch_cur(st);
	if (ch == EOF) {
		st->tok = T_EOF;
	} else if (isdigit(ch)) { /* number ::= [0-9]+ */
		st->tok = T_NUMBER;
		st->num_buf = 0;
		while (isdigit(ch)) {
			st->num_buf = (st->num_buf * 10) + (ch - '0');
			ch_next(st);
			ch = ch_cur(st);
		}
		fprintf(stderr, "TRACE:T_NUMBER=%ld\n", st->num_buf);
	} else if (isalpha(ch) || ch == '_') { /* identifier ::= [A-Za-z_][A-Za-z0-9_]* */
		unsigned cnt;

		cnt = 0;
		while (isalnum(ch) || ch == '_') {
			if (cnt > sizeof(st->id_buf) - 1) {
				error(st, "identifier too long");
				st->id_buf[0] = 0;
				return;
			}
			st->id_buf[cnt] = ch;
			cnt++;
			ch_next(st);
			ch = ch_cur(st);
		}
		st->id_buf[cnt] = 0;
		st->tok = T_IDENTIFIER;
	} else if (ch == '+') {
		st->tok = T_PLUS;
		ch_next(st); // TODO: move all these ch_next()'s up
	} else if (ch == '-') {
		st->tok = T_MINUS;
		ch_next(st);
	} else if (ch == '*') {
		st->tok = T_MUL;
		ch_next(st);
	} else if (ch == '/') {
		st->tok = T_DIV;
		ch_next(st);
	} else if (ch == '(') {
		st->tok = T_LEFT_PAREN;
		ch_next(st);
	} else if (ch == ')') {
		st->tok = T_RIGHT_PAREN;
		ch_next(st);
	} else {
		error(st, "unknown token");
		fprintf(stderr, "TRACE:%s():ch='%c'\n", __func__, ch);
	}
	fprintf(stderr, "TRACE:%s():tok=%d,error=%d\n", __func__, st->tok, st->error);
}

int tok_cur(struct pstate *st)
{
	return st->error ? T_EOF : st->tok;
}

void pstate_init(struct pstate *st)
{
	st->ch = '\n';
	st->tok = 0;
	st->error = 0;
	st->line = 1;
	st->offset = 0;
	st->offset = 0;
	st->num_buf = 0;
	st->id_buf[0] = 0;
	tok_next(st);
}

/*** AST ***/

enum ast_type {
	N_2OP, /* T_PLUS, T_MINUS, T_MUL, T_DIV */
	N_NUM, /* T_NUMBER */
	N_VAR, /* T_IDENTIFIER */
};

typedef struct ast_node *ast_node;
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

ast_node expr(struct pstate *st); /* forward */

ast_node ast_node_new(struct pstate *st, enum ast_type type)
{
	ast_node n;
	n = calloc(1, sizeof(*n));
	n->type = type;
	n->op = ~0;
	n->line = st->line;
	return n;
}

ast_node number(struct pstate *st)
{
	ast_node n;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	n = ast_node_new(st, N_NUM);
	n->num = st->num_buf;
	tok_next(st);
	return n;
}

ast_node identifier(struct pstate *st)
{
	ast_node n;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	n = ast_node_new(st, N_VAR);
	n->id = strdup(st->id_buf);
	tok_next(st);
	return n;
}

/* ExprParen ::= "(" Expr ")" */
ast_node expr_paren(struct pstate *st)
{
	ast_node n;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	tok_next(st);
	n = expr(st);
	if (!n)
		return NULL;
	if (tok_cur(st) != T_RIGHT_PAREN) {
		error(st, "missing parenthesis");
		return NULL;
	}
	tok_next(st);
	return n;
}

/* Factor ::= identifier | number | "(" Expr ")"
 */
ast_node factor(struct pstate *st)
{
	fprintf(stderr, "TRACE:%s()\n", __func__);
	if (tok_cur(st) == T_EOF) {
		fprintf(stderr, "<EOF>\n");
		return NULL; /* ast_node_new(st, T_EOF); */
	}
	if (tok_cur(st) == T_IDENTIFIER) {
		return identifier(st);
	} else if (tok_cur(st) == T_NUMBER) {
		return number(st);
	} else if (tok_cur(st) == T_LEFT_PAREN) { /* "(" Expr ")" */
		return expr_paren(st);
	}

	error(st, "missing identifier or number");
	return NULL;
}

/* Term ::= Factor { "*" Factor }
 */
ast_node term(struct pstate *st)
{
	ast_node left;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	left = factor(st);
	if (!left)
		return NULL;
	while (tok_cur(st) == T_MUL || tok_cur(st) == T_DIV) {
		ast_node new = ast_node_new(st, N_2OP);

		new->op = tok_cur(st);
		tok_next(st);
		new->left = left;
		new->right = factor(st);
		left = new; /* recurse left */
	}

	return left;
}

/* Expr ::= Term { "+" Term }
 */
ast_node expr(struct pstate *st)
{
	ast_node left;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	/* TODO: check for T_MINUS to find unary +/- */
	left = term(st);
	if (!left)
		return NULL;
	while (tok_cur(st) == T_PLUS || tok_cur(st) == T_MINUS) {
		ast_node new = ast_node_new(st, N_2OP);

		new->op = tok_cur(st);
		tok_next(st);
		new->left = left;
		new->right = term(st);
		left = new; /* recurse left */
	}

	return left;
}

/*** MAIN ***/

static const char *opname(int op)
{
	switch (op) {
	case T_PLUS: return "+";
	case T_MINUS: return "-";
	case T_MUL: return "*";
	case T_DIV: return "/";
	}
	return "UNKNOWN";
}

void dump(ast_node n)
{
	if (!n)
		return;

	switch (n->type) {
	case N_2OP:
		printf(" (%s", opname(n->op));
		dump(n->left);
		dump(n->right);
		printf(")");
		return;
	case N_NUM:
		printf(" %ld", n->num);
		return;
	case N_VAR:
		printf(" %s", n->id);
		return;
	}

	printf("ERROR\n");
}

int main()
{
	struct pstate st;
	ast_node root;

	pstate_init(&st);
	root = expr(&st);
	discard_whitespace(&st);
	if (tok_cur(&st) != T_EOF)
		error(&st, "trailing garbage");
	printf("\nDONE!\n");
	if (st.error) {
		return 1;
	}
	dump(root);
	printf("\n");
	/* TODO: ast_node_free(root); */
	return 0;
}
