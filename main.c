#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
			  TK_RESERVED,
			  TK_RETURN,
			  TK_IF,
			  TK_ELSE,
			  TK_WHILE,
			  TK_FOR,
			  TK_BREAK,
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

bool consume_kw(TokenKind kind) {
	if (token->kind == kind) {
		token = token->next;
		return true;
	}
	return false;
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

		if (strncmp(p, "return", 6) == 0 && !isalpha(*(p + 6))) {
			cur = new_token(TK_RETURN, cur, p, 6);
			p += 6;
			continue;
		}
		if (strncmp(p, "if", 2) == 0 && !isalpha(*(p + 2))) {
			cur = new_token(TK_IF, cur, p, 2);
			p += 2;
			continue;
		}
		if (strncmp(p, "else", 4) == 0 && !isalpha(*(p + 4))) {
			cur = new_token(TK_ELSE, cur, p, 4);
			p += 4;
			continue;
		}
		if (strncmp(p, "while", 5) == 0 && !isalpha(*(p + 5))) {
			cur = new_token(TK_WHILE, cur, p, 5);
			p += 5;
			continue;
		}
		if (strncmp(p, "for", 3) == 0 && !isalpha(*(p + 3))) {
			cur = new_token(TK_FOR, cur, p, 3);
			p += 3;
			continue;
		}
		if (strncmp(p, "break", 5) == 0 && !isalpha(*(p + 5))) {
			cur = new_token(TK_BREAK, cur, p, 5);
			p += 5;
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

		if (strchr("<>+-*/()=,;{}", *p)) {
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
		char symbol[256];
		if (tok->str) {
			if (tok->len < 255) {
				memcpy(symbol, tok->str, tok->len);
				symbol[tok->len] = '\0';
			} else {
				strncpy(symbol, "too long symbol", 255);
			}
		}
		
		switch (tok->kind) {
		case TK_RESERVED:
			printf("  TK_RESERVED %s\n", symbol);
			break;
		case TK_RETURN:
			printf("  TK_RETURN\n");
			break;
		case TK_IF:
			printf("  TK_IF\n");
			break;
		case TK_ELSE:
			printf("  TK_ELSE\n");
			break;
		case TK_WHILE:
			printf("  TK_WHILE\n");
			break;
		case TK_FOR:
			printf("  TK_FOR\n");
			break;
		case TK_BREAK:
			printf("  TK_BREAK\n");
			break;
		case TK_IDENT:
			printf("  TK_IDENT %s\n", symbol);
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
			  ND_EQ,       // ==
			  ND_NE,       // !=
			  ND_LT,       // <
			  ND_LE,       // <=
			  ND_ADD,      // +
			  ND_SUB,      // -
			  ND_MUL,      // *
			  ND_DIV,      // /
			  ND_ASSIGN,   // =
			  ND_LVAR,     // local variable
			  ND_NUM,      // integer
			  ND_FUNCCALL, // funccall

			  ND_EXPR_SENTINEL, // The above nodes are expression. Don't use this for any node kind.

			  ND_FUNCDEF, // function definition
			  ND_RETURN,  // return
			  ND_IF,      // if
			  ND_WHILE,   // while
			  ND_FOR,     // for
			  ND_BREAK,   // break
			  ND_BLOCK,   // block
} NodeKind;

bool is_expr_node(NodeKind kind) {
	if (kind < ND_EXPR_SENTINEL) {
		return true;
	}
	return false;
}

typedef struct Node Node;

// Node represents a node of an AST.
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	Node *opt1;
	Node *opt2;
	int val;
	char *str;
	int offset;
	int label_num;
	int func_id;
	char *func_name;
	Node *next;
};

void print_node(Node *node, int depth, char *prefix) {
	if (!node) {
		return;
	}

	for (int i = 0; i < depth; i++) {
		printf("  ");
	}
	if (prefix) {
		printf("%s: ", prefix);
	}
	
	switch (node->kind) {
	case ND_EQ:
		printf("==\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_NE:
		printf("!=\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_LT:
		printf("<\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_LE:
		printf("<=\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_ADD:
		printf("+\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_SUB:
		printf("-\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_MUL:
		printf("*\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_DIV:
		printf("/\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_ASSIGN:
		printf("=\n");
		print_node(node->lhs, depth + 1, "LHS");
		print_node(node->rhs, depth + 1, "RHS");
		break;
	case ND_LVAR:
		printf("VARIABLE: (offset) %d\n", node->offset);
		break;
	case ND_NUM:
		printf("NUMBER: %d\n", node->val);
		break;
	case ND_FUNCCALL:
		printf("CALL: %s\n", node->func_name);
		print_node(node->lhs, depth + 1, "PARAMETER");
		break;
	case ND_FUNCDEF:
		printf("FUNCTION: #%d %s\n", node->func_id, node->func_name);
		print_node(node->lhs, depth + 1, "ARGUMENT");
		print_node(node->rhs, depth + 1, NULL);
		break;
	case ND_RETURN:
		printf("RETURN\n");
		print_node(node->lhs, depth + 1, NULL);
		break;
	case ND_IF:
		printf("IF\n");
		print_node(node->lhs, depth + 1, "CONDITION");
		print_node(node->rhs, depth + 1, "IF CLAUSE");
		print_node(node->opt1, depth + 1, "ELSE CLAUSE");
		break;
	case ND_WHILE:
		printf("WHILE\n");
		print_node(node->lhs, depth + 1, "CONDITION");
		break;
	case ND_FOR:
		printf("FOR\n");
		print_node(node->lhs, depth + 1, "INIT");
		print_node(node->rhs, depth + 1, "CONDITION");
		print_node(node->opt1, depth + 1, "INCREMENT");
		print_node(node->opt2, depth + 1, "BODY");
		break;
	case ND_BREAK:
		printf("BREAK\n");
		break;
	case ND_BLOCK:
		printf("BLOCK\n");
		print_node(node->lhs, depth + 1, NULL);
		break;
	default:
		printf("UNKNOWN NODE KIND: %d\n", node->kind);
		break;
	}

	print_node(node->next, depth, prefix);
}

void print_code(Node **code) {
	for (int i = 0; code[i]; i++) {
		print_node(code[i], 0, NULL);
	}
}

// new_node returns a new node.
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

// new_node_if returns a new if statment node.
Node *new_node_if(Node *cond, Node *true_stmt, Node *false_stmt) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_IF;
	node->lhs = cond;
	node->rhs = true_stmt;
	node->opt1 = false_stmt;
	return node;
}

// new_node_for returns a new for statment node.
Node *new_node_for(Node *init, Node *cond, Node *increment, Node *stmt) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_FOR;
	node->lhs = init;
	node->rhs = cond;
	node->opt1 = increment;
	node->opt2 = stmt;
	return node;
}

// new_node_num returns a new node that represents an integer.
Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

// new_node_funccall returns a new function call node.
Node *new_node_funccall(char *func_name, Node *params) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_FUNCCALL;
	node->func_name = func_name;
	node->lhs = params;
	return node;
}

typedef struct LVar LVar;

struct LVar {
	LVar *next;
	char *name;
	int len;
	int offset;
};

int func_id;
LVar *locals[100];

LVar *find_lvar(int func_id, Token *tok) {
	for (LVar *var = locals[func_id]; var; var = var->next) {
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
			return var;
		}
	}
	return NULL;
}

void program();
Node *func_def();
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
int label_num;

// program = func_def+
void program() {
	int i = 0;
	while (!at_eof()) {
		code[i++] = func_def();
	}
	code[i] = NULL;

	if (code[0] == NULL) {
		error("expected function definition at least one");
	}
}

// func_def: ident "(" (ident ("," ident)*)? ")" "{" stmt* "}"
Node *func_def() {
	Token *id_tok = consume_ident();
	if (!id_tok) {
		error("expected an identifier");
	}

	expect("(");
	Node args;
	args.next = NULL;
	Node *arg = &args;
	while (!consume(")")) {
		Token *arg_tok = consume_ident();
		if (!arg_tok) {
			error("expected an identifier");
		}

		Node *decl = calloc(1, sizeof(Node));
		decl->kind = ND_LVAR;

		LVar *lvar = find_lvar(func_id, arg_tok);
		if (lvar) {
			decl->offset = lvar->offset;
		} else {
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals[func_id];
			lvar->name = arg_tok->str;
			lvar->len = arg_tok->len;
			if (locals[func_id]) {
				lvar->offset = locals[func_id]->offset + 8;
			} else {
				lvar->offset = 8;
			}
			locals[func_id] = lvar;
			decl->offset = lvar->offset;
		}

		arg->next = decl;
		arg = arg->next;
		if (consume(")")) {
			break;
		}
		expect(",");
	}

	expect("{");
	Node block_head;
	block_head.next = NULL;
	Node *stmt_node = &block_head;
	while(!consume("}")) {
		stmt_node->next = stmt();
		stmt_node = stmt_node->next;
	}
	Node *block_node = new_node(ND_BLOCK, block_head.next, NULL);

	char *func_name = calloc(id_tok->len + 1, sizeof(char));
	memcpy(func_name, id_tok->str, id_tok->len);
	func_name[id_tok->len] = '\0';
	
	Node *node = new_node(ND_FUNCDEF, args.next, block_node);
	node->func_id = func_id++;
	node->func_name = func_name;

	return node;
}

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "break" ";"
//      | "{" stmt* "}"
Node *stmt() {
	if (consume_kw(TK_RETURN)) {
		Node *node = new_node(ND_RETURN, expr(), NULL);
		expect(";");
		return node;
	} else if (consume_kw(TK_IF)) {
		expect("(");
		Node *cond_node = expr();
		expect(")");
		Node *true_stmt_node = stmt();
		Node *false_stmt_node = NULL;
		if (consume_kw(TK_ELSE)) {
			false_stmt_node = stmt();
		}
		Node *if_node = new_node_if(cond_node, true_stmt_node, false_stmt_node);
		if_node->label_num = label_num++;
		return if_node;
	} else if (consume_kw(TK_WHILE)) {
		expect("(");
		Node *cond_node = expr();
		expect(")");
		Node *while_node = new_node(ND_WHILE, cond_node, stmt());
		while_node->label_num = label_num++;
		return while_node;
	} else if (consume_kw(TK_FOR)) {
		expect("(");
		Node *init_node = NULL;
		if (!consume(";")) {
			init_node = expr();
			expect(";");
		}
		Node *cond_node = NULL;
		if (!consume(";")) {
			cond_node = expr();
			expect(";");
		}
		Node *increment_node = NULL;
		if (!consume(")")) {
			increment_node = expr();
			expect(")");
		}
		Node *for_node = new_node_for(init_node, cond_node, increment_node, stmt());
		for_node->label_num = label_num++;
		return for_node;
	} else if (consume_kw(TK_BREAK)) {
		Node *node = new_node(ND_BREAK, NULL, NULL);
		expect(";");
		return node;
	} else if (consume("{")) {
		Node head;
		head.next = NULL;
		Node *stmt_node = &head;
		while(!consume("}")) {
			stmt_node->next = stmt();
			stmt_node = stmt_node->next;
		}
		return new_node(ND_BLOCK, head.next, NULL);
	}
	
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

// primary = "(" expr ")"
//         | ident ("(" (expr ("," expr)*)? ")")?
//         | num
Node *primary() {
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

	Token *tok = consume_ident();
	if (tok) {
		if (consume("(")) {
			Node params;
			params.next = NULL;
			Node *p = &params;
			while (!consume(")")) {
				p->next = expr();
				p = p->next;
				if (consume(")")) {
					break;
				}
				expect(",");
			};
			char *func_name = calloc(tok->len + 1, sizeof(char));
			memcpy(func_name, tok->str, tok->len);
			func_name[tok->len] = '\0';
			return new_node_funccall(func_name, params.next);
		}
		
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(func_id, tok);
		if (lvar) {
			node->offset = lvar->offset;
		} else {
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals[func_id];
			lvar->name = tok->str;
			lvar->len = tok->len;
			if (locals[func_id]) {
				lvar->offset = locals[func_id]->offset + 8;
			} else {
				lvar->offset = 8;
			}
			locals[func_id] = lvar;
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
void gen(Node *node, char *breakLabel) {
	if (node == NULL) {
		return;
	}

	switch (node->kind) {
	case ND_NUM:
		printf("  push %d\n", node->val);
		return;
	case ND_FUNCCALL: {
		int nth = 1;
		for (Node *param = node->lhs; param; param = param->next) {
			gen(param, NULL);
			switch (nth) {
			case 1:
				printf("  pop rdi\n");
				break;
			case 2:
				printf("  pop rsi\n");
				break;
			case 3:
				printf("  pop rdx\n");
				break;
			case 4:
				printf("  pop rcx\n");
				break;
			case 5:
				printf("  pop r8\n");
				break;
			case 6:
				printf("  pop r9\n");
				break;
			}
			nth++;
		}
		printf("  call %s\n", node->func_name);
		printf("  push rax\n");
		return;
	}
	case ND_LVAR:
		gen_lval(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	case ND_ASSIGN:
		gen_lval(node->lhs);
		gen(node->rhs, breakLabel);
		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
		return;
	case ND_RETURN:
		gen(node->lhs, breakLabel);
		printf("  pop rax\n");
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
		return;
	case ND_IF:
		// lhs: condition
		// rhs: statement to execute when condition is true (if clause)
		// opt1: statement to execute when condition is false (else clause) (optional)
		gen(node->lhs, breakLabel);
		if (node->opt1) {
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je .Lelse%d\n", node->label_num);
			gen(node->rhs, breakLabel);
			printf("  jmp .Lend%d\n", node->label_num);
			printf(".Lelse%d:\n", node->label_num);
			gen(node->opt1, breakLabel);
			printf(".Lend%d:\n", node->label_num);
		} else {
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je .Lend%d\n", node->label_num);
			gen(node->rhs, breakLabel);
			printf(".Lend%d:\n", node->label_num);
		}
		return;
	case ND_WHILE: {
		char breakLabel[256];
		sprintf(breakLabel, ".Lend%d", node->label_num);
		
		// lhs: condition
		// rhs: statement to execute when condition is true
		printf(".Lbegin%d:\n", node->label_num);
		gen(node->lhs, breakLabel);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je %s\n", breakLabel);
		gen(node->rhs, breakLabel);
		printf("  jmp .Lbegin%d\n", node->label_num);
		printf("%s:\n", breakLabel);
		return;
	}
	case ND_FOR: {
		char breakLabel[256];
		sprintf(breakLabel, ".Lend%d", node->label_num);
		
		// lhs: init (optional)
		// rhs: condition (optional)
		// opt1: increment (optional)
		// opt2: statement to execute when condition is true
		gen(node->lhs, breakLabel);
		printf(".Lbegin%d:\n", node->label_num);
		if (node->rhs) {
			gen(node->rhs, breakLabel);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je %s\n", breakLabel);
		}
		gen(node->opt2, breakLabel);
		gen(node->opt1, breakLabel);
		printf("  jmp .Lbegin%d\n", node->label_num);
		// If the condition expression is missing, it seems that this label isn't required.
		// But when the break statement is used in this for statement, this label is required to break from it.
		printf("%s:\n", breakLabel);
		return;
	}
	case ND_BREAK:
		if (breakLabel == NULL || strlen(breakLabel) <= 0) {
			error("`break` can only be used in for or while statement.");
		}
		printf("  jmp %s\n", breakLabel);
		return;
	case ND_BLOCK:
		// lhs: list of statements
		for (Node *stmt = node->lhs; stmt; stmt = stmt->next) {
			gen(stmt, breakLabel);
			// If `node` is an expression, discards its result.
			if (is_expr_node(stmt->kind)) {
				printf("  pop rax\n");
			}
		}
		return;
	}

	gen(node->lhs, NULL);
	gen(node->rhs, NULL);

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
	//	printf("# tokenizing start\n");
	token = tokenize();
	//	printf("# tokenizing finished\n");
	//	print_tokens();
	//	printf("# parsing start\n");
	program();
	//	printf("# parsing finished\n");
	//	print_code(code);

	//	printf("# code generation start\n");

	bool main_found = false;
	for (int i = 0; code[i]; i++) {
		Node *node = code[i];
		if (node->kind != ND_FUNCDEF) {
			continue;
		}
		if (strncmp(node->func_name, "main", 4) == 0) {
			main_found = true;
			break;
		}
	}
	if (!main_found) {
		error("main function is not found");
	}

	printf(".intel_syntax noprefix\n");

	for (int i = 0; code[i]; i++) {
		Node *node = code[i];
		if (node->kind != ND_FUNCDEF) {
			continue;
		}

		printf(".global %s\n", node->func_name);
		printf("%s:\n", node->func_name);
		printf("  push rbp\n");
		printf("  mov rbp, rsp\n");

		if (locals[node->func_id]) {
			printf("  sub rsp, %d\n", locals[node->func_id]->offset);
		}

		int nth = 1;
		for (Node *arg = node->lhs; arg; arg = arg->next) {
			gen_lval(arg);
			printf("  pop rax\n");
			
			switch (nth) {
			case 1:
				printf("  mov [rax], rdi\n");
				break;
			case 2:
				printf("  mov [rax], rsi\n");
				break;
			case 3:
				printf("  mov [rax], rdx\n");
				break;
			case 4:
				printf("  mov [rax], rcx\n");
				break;
			case 5:
				printf("  mov [rax], r8\n");
				break;
			case 6:
				printf("  mov [rax], r9\n");
				break;
			}
			nth++;
		}

		gen(node->rhs, NULL);

		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
	}

	//	printf("# code generation finished\n");
	
	return 0;
}
