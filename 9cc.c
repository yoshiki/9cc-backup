#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1

// Kind of token
typedef enum {
  TK_RESERVED, // Symbols
  TK_NUM,      // Integer
  TK_EOF,      // Token of the end of file
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind; // Type of token
  Token *next;    // Next token
  int val;        // Integer when kind is TK_NUM
  char *str;      // Token string
  int len;        // Token length
};

// Current token
Token *currentToken;

// Kind of node for abstract syntax tree
typedef enum {
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_EQ,   // ==
  ND_NE,   // !=
  ND_LT,   // <
  ND_LE,   // <=
  //ND_GT,   // > is invert to ND_LT
  //ND_GE,   // >= is invert to ND_LE
  ND_NUM,  // Integer
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;  // Type of node
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  int val;        // Use only the kind is ND_NUM
};

// Print error function
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Input program
char *user_input;

// Print error to point out
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Go forward to the next token when current token is expected symbol
// and return true, else return false.
bool consume(char *op) {
  if (currentToken->kind != TK_RESERVED ||
    strlen(op) != currentToken->len ||
    memcmp(currentToken->str, op, currentToken->len))
    return false;
  currentToken = currentToken->next;
  return true;
}

// Go forward to the next token when current token is expected symbol,
// else output error.
void expect(char *op) {
  if (currentToken->kind != TK_RESERVED ||
    strlen(op) != currentToken->len ||
    memcmp(currentToken->str, op, currentToken->len))
    error_at(currentToken->str, "Not '%c'", op);
  currentToken = currentToken->next;
}

// Go forward to the next token and return the value
// when current token is expected symbol, else output error.
int expect_number() {
  if (currentToken->kind != TK_NUM)
    error_at(currentToken->str, "Not integer");
  int val = currentToken->val;
  currentToken = currentToken->next;
  return val;
}

bool at_eof() {
  return currentToken->kind == TK_EOF;
}

// Create new token and connect it to cur
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

// Tokenize input string(*p) and return it
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip space
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "Invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// Create new node
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// Create new node for integer
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// expr       = equality
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | "(" expr ")"

Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *expr();

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  } else {
    return new_node_num(expect_number());
  }
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), unary());
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *expr() {
  return equality();
}

void print_space(int num) {
  if (num == 0) return;
  int padding = 2;
  for (int i = 0; i < (num - 1) * padding; i++) {
    printf(" ");
  }
  for (int i = 0; i < padding; i++) {
    if (i == 0)
      printf("└");
    else
      printf("─");
  }
}

void dump_node(Node *node, int depth) {
  print_space(depth);

  if (node->kind == ND_NUM) {
    printf("%d\n", node->val);
    return;
  }

  switch (node->kind) {
    case ND_ADD:
      printf("+");
      break;
    case ND_SUB:
      printf("-");
      break;
    case ND_MUL:
      printf("*");
      break;
    case ND_DIV:
      printf("/");
      break;
    case ND_EQ:
      printf("==");
      break;
    case ND_NE:
      printf("!=");
      break;
    case ND_LT:
      printf("<");
      break;
    case ND_LE:
      printf("<=");
      break;
  }
  printf("\n");

  depth++;
  dump_node(node->lhs, depth);
  dump_node(node->rhs, depth);
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
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
      printf("  idiv rdi\n");
      break;
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
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: Invalid number of arguments\n", argv[0]);

  // Get user input
  user_input = argv[1];

  // Do tokenize
  currentToken = tokenize(argv[1]);

  // Parse
  Node *node = expr();

  // Dump node
  if (DEBUG) {
    dump_node(node, 0);
    return 0;
  }

  // Output header of assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Generate code while traversing AST to emit assembly
  gen(node);

  // Load to rax from top of stach(It's result) and return
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;
}
