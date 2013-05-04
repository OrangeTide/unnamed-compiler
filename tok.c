/* tok.c : lexer turns input into tokens */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "tok.h"
#include "trace.h"

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
	fprintf(stderr, "ERROR:line=%d,ofs=%d:%s\n", st->line, st->offset, reason);
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

/* TODO: return an error string */
int last_error(struct pstate *st)
{
	return st->error;
}

int line_cur(struct pstate *st)
{
	return st->line;
}

long num_buf(struct pstate *st)
{
	return st->num_buf;
}

const char *id_buf(struct pstate *st)
{
	return st->id_buf;
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
	TRACE;
	while (isspace(ch_cur(st)))
		ch_next(st);
}

void tok_next(struct pstate *st)
{
	char ch;

	TRACE;
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
		TRACE_FMT("T_NUMBER=%ld\n", st->num_buf);
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
		TRACE_FMT("%s():ch='%c'\n", __func__, ch);
	}
	TRACE_FMT("%s():tok=%d,error=%d\n", __func__, st->tok, st->error);
}

int tok_cur(struct pstate *st)
{
	return st->error ? T_EOF : st->tok;
}

struct pstate *pstate_new(void)
{
	struct pstate *st;

	st = calloc(1, sizeof(*st));
	st->ch = '\n';
	st->line = 1;
	tok_next(st);
	return st;
}

void pstate_free(struct pstate *st)
{
	free(st);
}