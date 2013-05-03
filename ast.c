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

struct pstate {
	int ch;
	int error;
};

void ch_next(struct pstate *st)
{
	if (st->ch == EOF)
		return;
	st->ch = getchar();
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
	while (*str && require(st, *str))
			str++;
	if (*str != 0)  /* else we could not consume the entire string, which is bad and we must give up! */
		st->error = 1;
	return !st->error;
}

void pstate_init(struct pstate *st)
{
	st->ch = '\n';
	st->error = 0;
}

/* parse trees...
 * (+ (- a b) (- c d))
 * root -> add(sub(a, b), sub(c, d))
 */
int main()
{
	struct pstate ps;

	pstate_init(&ps);

	return 0;
}
