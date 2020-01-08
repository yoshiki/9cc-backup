#include "9cc.h"

bool at_eof() {
  return currentToken->kind == TK_EOF;
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

// Go forward to the next token and return current token
// when current token is expected symbol.
Token *consume_ident() {
  if (currentToken->kind != TK_INDENT)
    return NULL;
  Token *t = currentToken;
  currentToken = currentToken->next;
  return t;
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

// program    = stmt*
// stmt       = expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | ident | "(" expr ")"

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok;
  if (tok = consume_ident()) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }

  return new_node_num(expect_number());
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

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

Node *code[100];

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}
