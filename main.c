#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
			  TK_RESERVED,
			  TK_NUM,
			  TK_EOF,
} TokenKind;

typedef struct Token Token;

// Token represents a token.
struct Token {
	TokenKind kind;
	Token *next;
	int val;
	char *str;
};

// a token now focused on
Token *token;

// error reports an error and exits with exit code 1.
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// consume  consumes a token and returns true when the the token now focused on is
// equal to specified symbol. Otherwise, don't consume and returns false.
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		return false;
	}
	token = token->next;
	return true;
}

// expect checks whether the token now focused on is equal to a specified symbol.
// If the check passed, it consumes the token. Otherwise, it reports an error and exits.
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		error("expected: '%c'", op);
	}
	token = token->next;
}

// expect_number checks whether the token now focused on is a number symbol.
// If the check passed, it consumes the token and returns the value.
// Otherwise, it reports an error and exits.
int expect_number() {
	if (token->kind != TK_NUM) {
		error("expected: a number, actual: not a number");
	}
	int val = token->val;
	token = token->next;
	return val;
}

// at_eof checks whether the token now focused on is the EOF.
bool at_eof() {
	return token->kind == TK_EOF;
}

// new_token returns a new token and links it to cur.
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// tokenize tokenizes a string passed.
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error("failed to tokenize");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "n9cc is required 2 parameters");
		return 1;
	}

	token = tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");
	
	printf("  mov rax, %d\n", expect_number());

	while (!at_eof()) {
		if (consume('+')) {
			printf("  add rax, %d\n", expect_number());
			continue;
		}
		
		expect('-');
		printf("  sub rax, %d\n", expect_number());
	}
	
	printf("  ret\n");
	return 0;
}
