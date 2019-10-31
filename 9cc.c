#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
};

// Current token
Token *currentToken;

// Kind of node for abstract syntax tree
typedef enum {
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
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
bool consume(char op) {
  if (currentToken->kind != TK_RESERVED || currentToken->str[0] != op)
    return false;
  currentToken = currentToken->next;
  return true;
}

// Go forward to the next token when current token is expected symbol,
// else output error.
void expect(char op) {
  if (currentToken->kind != TK_RESERVED || currentToken->str[0] != op)
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
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
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

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "Invalid token");
  }

  new_token(TK_EOF, cur, p);
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

// expr    = mul ("+" mul | "-" mul)*
// mul     = primary ("*" primary | "/" primary)*
// primary = num | "(" expr ")"

Node *primary();
Node *mul();
Node *expr();

Node *primary() {
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  } else {
    return new_node_num(expect_number());
  }
}

Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*'))
      node = new_node(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_node(ND_DIV, node, primary());
    else
      return node;
  }
}

Node *expr() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
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
