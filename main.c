#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
			  TK_RESERVED,
			  TK_IDENT,
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
	int len;
};

// a token now focused on
Token *token;

// the entier input source code
char *user_input;

// error reports an error and exits with exit code 1.
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// error reports an error and exits with exit code 1.
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// consume consumes a token and returns true when the the token now focused on is
// equal to specified symbol. Otherwise, don't consume and returns false.
bool consume(char *op) {
	if (token->kind != TK_RESERVED || token->len != strlen(op)
		|| memcmp(token->str, op, token->len)) {
		return false;
	}
	token = token->next;
	return true;
}

// consume consumes a token and returns it when the the token now focused on is
// an identifier. Otherwise, don't consume and returns NULL.
Token *consume_ident() {
	if (token->kind != TK_IDENT) {
		return NULL;
	}
	Token *tok = token;
	token = token->next;
	return tok;
}

// expect checks whether the token now focused on is equal to a specified symbol.
// If the check passed, it consumes the token. Otherwise, it reports an error and exits.
void expect(char *op) {
	if (token->kind != TK_RESERVED || token->len != strlen(op)
		|| memcmp(token->str, op, token->len)) {
		error_at(token->str, "expected '%s'", op);
	}
	token = token->next;
}

// expect_number checks whether the token now focused on is a number symbol.
// If the check passed, it consumes the token and returns the value.
// Otherwise, it reports an error and exits.
int expect_number() {
	if (token->kind != TK_NUM) {
		error_at(token->str, "expected a number");
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

// tokenize tokenizes `user_input`.
Token *tokenize() {
	Token head;
	head.next = NULL;
	Token *cur = &head;
	char *p = user_input;

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (isalpha(*p)) {
			char *start = p;
			int len = 0;
			do {
				len++;
				p++;
			} while (isalnum(*p));
			cur = new_token(TK_IDENT, cur, start, len);
			continue;
		}

		if (startswith(p, "==") || startswith(p, "!=")
			|| startswith(p, "<=") || startswith(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strchr("<>+-*/()=;", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

void print_tokens() {
	printf("tokens:\n");
	for (Token *tok = token; tok != NULL; tok = tok->next) {
		switch (tok->kind) {
		case TK_RESERVED:
			printf("  TK_RESERVED\n");
			break;
		case TK_NUM:
			printf("  TK_NUM %d\n", tok->val);
			break;
		case TK_EOF:
			printf("  TK_EOF\n");
			break;
		}
	}
}

typedef enum {
			  ND_EQ,     // ==
			  ND_NE,     // !=
			  ND_LT,     // <
			  ND_LE,     // <=
			  ND_ADD,    // +
			  ND_SUB,    // -
			  ND_MUL,    // *
			  ND_DIV,    // /
			  ND_ASSIGN, // =
			  ND_LVAR,   // local variable
			  ND_NUM,    // integer
} NodeKind;

typedef struct Node Node;

// Node represents a node of an AST.
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
	int offset;
};

// new_node returns a new node.
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

// new_node returns a new node that represents an integer.
Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

typedef struct LVar LVar;

struct LVar {
	LVar *next;
	char *name;
	int len;
	int offset;
};

LVar *locals;

LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next) {
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
			return var;
		}
	}
	return NULL;
}

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *code[100];

// program = expr*
void program() {
	int i = 0;
	while (!at_eof()) {
		code[i++] = stmt();
	}
	code[i] = NULL;
}

// stmt = expr ";"
Node *stmt() {
	Node *node = expr();
	expect(";");
	return node;
}

// expr = equality
Node *expr() {
	return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
	Node *node = equality();

	if (consume("=")) {
		node = new_node(ND_ASSIGN, node, assign());
	}
	return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume("==")) {
			node = new_node(ND_EQ, node, relational());
		} else if (consume("!=")) {
			node = new_node(ND_NE, node, relational());
		} else {
			return node;
		}
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume("<")) {
			node = new_node(ND_LT, node, add());
		} else if (consume("<=")) {
			node = new_node(ND_LE, node, add());
		} else if (consume(">")) {
			node = new_node(ND_LT, add(), node);
		} else if (consume(">=")) {
			node = new_node(ND_LE, add(), node);
		} else {
			return node;
		}
	}
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume("+")) {
			node = new_node(ND_ADD, node, mul());
		} else if (consume("-")) {
			node = new_node(ND_SUB, node, mul());
		} else {
			return node;
		}
	}
}

// mul = primary ("*" unary | "/" unary)*
Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*")) {
			node = new_node(ND_MUL, node, unary());
		} else if (consume("/")) {
			node = new_node(ND_DIV, node, unary());
		} else {
			return node;
		}
	}
}

// unary = ("+" | "-")? unary
//       | primary
Node *unary() {
	if (consume("+")) {
		return unary();
	}
	if (consume("-")) {
		return new_node(ND_SUB, new_node_num(0), unary());
	}
	return primary();
}

// primary = "(" expr ")" | num
Node *primary() {
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

	Token *tok = consume_ident();
	if (tok) {
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(tok);
		if (lvar) {
			node->offset = lvar->offset;
		} else {
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			if (locals) {
				lvar->offset = locals->offset + 8;
			} else {
				lvar->offset = 8;
			}
			locals = lvar;
			node->offset = lvar->offset;
		}

		return node;
	}

	return new_node_num(expect_number());
}

void gen_lval(Node *node) {
	if (node->kind != ND_LVAR) {
		error("left value must be a variable");
	}

	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", node->offset);
	printf("  push rax\n");
}

// gen generates asembly.
void gen(Node *node) {
	switch (node->kind) {
	case ND_NUM:
		printf("  push %d\n", node->val);
		return;
	case ND_LVAR:
		gen_lval(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	case ND_ASSIGN:
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind) {
	case ND_EQ:
		printf("  cmp rax, rdi\n");
		printf("  sete al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_NE:
		printf("  cmp rax, rdi\n");
		printf("  setne al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LT:
		printf("  cmp rax, rdi\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LE:
		printf("  cmp rax, rdi\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_ADD:
		printf("  add rax, rdi\n");
		break;
	case ND_SUB:
		printf("  sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("  imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("  cqo\n");
		printf("  idiv rax, rdi\n");
		break;
	}

	printf("  push rax\n");
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "n9cc is required 2 parameters");
		return 1;
	}
	
	user_input = argv[1];
	token = tokenize();
	//	print_tokens();
	program();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	
	if (locals) {
		printf("  sub rsp, %d\n", locals->offset);
	}

	for (int i = 0; code[i]; i++) {
		gen(code[i]);

		printf("  pop rax\n");
	}

	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
	return 0;
}
