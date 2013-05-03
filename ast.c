/*
 * Expr ::= Term
 * Term ::= Term { '+' Factor } | Factor
 * Factor ::= Factor { '*' identifier } | identifier
 * identifier ::= [a-z]
 *
 * OLD:
 * sum ::= term op term
 * op ::= '+' | '-' | '*' | '/'
 * term ::= num | id
 * id ::= [a-z]
 *
 * Future:
 * assignStmt := lvalue = expr
 */
/* LINKS:
 * http://en.wikipedia.org/wiki/Tail_recursive_parser
 */
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
	} else if (isdigit(ch)) {
		st->tok = T_NUMBER;
		st->num_buf = ch - '0';
		/* TODO: loop through number */
		fprintf(stderr, "TRACE:T_NUMBER=%ld\n", st->num_buf);
		ch_next(st);
	} else if (isalpha(ch) || ch == '_') {
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
		ch_next(st);
	} else if (ch == '*') {
		st->tok = T_MUL;
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
typedef struct ast_node *ast_node;
struct ast_node {
	int op;
	ast_node left;
	union {
		ast_node right; /* T_PLUS, T_MINUS, T_MUL, T_DIV */
		long num; /* T_NUMBER */
		char *id; /* T_IDENTIFIER */
	};
};

ast_node ast_node_new(int op)
{
	ast_node n;
	n = calloc(1, sizeof(*n));
	n->op = op;
	return n;
}

ast_node identifier(struct pstate *st)
{
	ast_node n;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	if (tok_cur(st) == T_EOF) {
		fprintf(stderr, "<EOF>\n");
		return NULL; /* ast_node_new(T_EOF); */
	}
	if (tok_cur(st) == T_IDENTIFIER) {
		n = ast_node_new(T_IDENTIFIER);
		n->id = strdup(st->id_buf);
		tok_next(st);
	} else if (tok_cur(st) == T_NUMBER) {
		n = ast_node_new(T_NUMBER);
		n->num = st->num_buf;
		tok_next(st);
	} else {
		error(st, "missing identifier or number");
		return NULL;
	}
	return n;
}

ast_node factor(struct pstate *st)
{
	ast_node left;

	fprintf(stderr, "TRACE:%s()\n", __func__);
	fprintf(stderr, "TRACE:%s()\n", __func__);
	left = identifier(st);
	if (!left)
		return NULL;
	while (tok_cur(st) == T_MUL || tok_cur(st) == T_DIV) {
		ast_node new = ast_node_new(tok_cur(st));

		tok_next(st);
		new->left = left;
		new->right = identifier(st);
		if (new->right == NULL) {
			error(st, "missing identifier or number");
			/* TODO: ast_node_free(new); ast_node_free(left) */
			return NULL;
		}
		left = new; /* recurse left */
	}

	return left;
}

ast_node term(struct pstate *st)
{
	ast_node left;

	left = factor(st);
	if (!left)
		return NULL;
	while (tok_cur(st) == T_PLUS || tok_cur(st) == T_MINUS) {
		ast_node new = ast_node_new(tok_cur(st));

		tok_next(st);
		new->left = left;
		new->right = factor(st);
		left = new; /* recurse left */
	}

	return left;
}

ast_node expr(struct pstate *st)
{
	return term(st);
}

/*** MAIN ***/

void dump(ast_node n)
{
	if (!n)
		return;
	printf(" (OP%d", n->op);
	dump(n->left);
	dump(n->right);
	printf(")");
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
