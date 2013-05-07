/* tok.c : lexer turns input into tokens. */
/*
 * Copyright (c) 2013 Jon Mayo <jon@rm-f.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

/* number ::= [0-9]+ */
void parse_number(struct pstate *st)
{
	int ch = ch_cur(st);

	st->tok = T_NUMBER;
	st->num_buf = 0;
	while (isdigit(ch)) {
		st->num_buf = (st->num_buf * 10) + (ch - '0');
		ch_next(st);
		ch = ch_cur(st);
	}
	TRACE_FMT("T_NUMBER=%ld\n", st->num_buf);
}

/* identifier or a keyword ("if", "then", "else", etc)
 * identifier ::= [A-Za-z_][A-Za-z0-9_]*
 */
void parse_identifier(struct pstate *st)
{
	unsigned cnt;
	int ch = ch_cur(st);

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
	if (!strcmp(st->id_buf, "if")) {
		TRACE_FMT("T_IF\n");
		st->tok = T_IF;
		st->id_buf[0] = 0;
	} else if (!strcmp(st->id_buf, "then")) {
		TRACE_FMT("T_THEN\n");
		st->tok = T_THEN;
		st->id_buf[0] = 0;
	} else if (!strcmp(st->id_buf, "else")) {
		TRACE_FMT("T_ELSE\n");
		st->tok = T_ELSE;
		st->id_buf[0] = 0;
	} else {
		TRACE_FMT("T_IDENTIFIER\n");
		st->tok = T_IDENTIFIER;
	}
}

void tok_next(struct pstate *st)
{
	char ch;

	TRACE;
	discard_whitespace(st); /* TODO: is this correct?? */
	ch = ch_cur(st);
	if (ch == EOF) {
		st->tok = T_EOF;
	} else if (isdigit(ch)) {
		parse_number(st);
	} else if (isalpha(ch) || ch == '_') {
		parse_identifier(st);
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
