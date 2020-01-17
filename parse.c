#include "9cc.h"

// Local variables
Var *locals;

bool at_eof() { return currentToken->kind == TK_EOF; }

// Go forward to the next token when current token is expected symbol
// and return true, else return false.
bool consume(char *op) {
  if (currentToken->kind != TK_RESERVED || strlen(op) != currentToken->len ||
      memcmp(currentToken->str, op, currentToken->len))
    return false;
  currentToken = currentToken->next;
  return true;
}

// Go forward to the next token and return current token
// when current token is expected symbol.
Token *consume_ident() {
  if (currentToken->kind != TK_INDENT) return NULL;
  Token *t = currentToken;
  currentToken = currentToken->next;
  return t;
}

// Go forward to the next token when current token is expected symbol,
// else output error.
void expect(char *op) {
  if (currentToken->kind != TK_RESERVED || strlen(op) != currentToken->len ||
      memcmp(currentToken->str, op, currentToken->len))
    error_at(currentToken->str, "Not '%s'", op);
  currentToken = currentToken->next;
}

// Go forward to the next token and return the value
// when current token is expected symbol, else output error.
int expect_number() {
  if (currentToken->kind != TK_NUM) error_at(currentToken->str, "Not integer");
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

// Create new node for ident
Node *new_node_var(Var *var) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->var = var;
  return node;
}

// Create new local var
Var *new_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = locals;
  locals = var;
  return var;
}

// Find variables with name.
// Return NULL if not found.
Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next) {
    if (var->name && !strncmp(tok->str, var->name, tok->len))
      return var;
  }
  return NULL;
}

// Get function args
Node *func_args() {
  if (consume(")")) return NULL;

  Node *head = assign();
  Node *cur = head;
  while (consume(",")) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return head;
}

// program    = stmt*
// stmt       = expr ";"
//              | "return" expr ";"
//              | "if" "(" expr ")" stmt ("else" stmt)?
//              | "while" "(" expr ")" stmt
//              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//              | "{" stmt* "}"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num
//              | ident ("(" (assign ("," assign)*)? ")")?
//              | "(" expr ")"

Function *program() {
  // Create first local variables
  locals = NULL;

  Node head = {};
  Node *cur = &head;

  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }

  Function *prog = calloc(1, sizeof(Function));
  prog->node = head.next;
  prog->locals = locals;

  return prog;
}

Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = new_node(ND_RETURN, expr(), NULL);
    expect(";");
  } else if (consume("if")) {
    node = new_node(ND_IF, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else")) node->els = stmt();
  } else if (consume("while")) {
    node = new_node(ND_WHILE, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
  } else if (consume("for")) {
    node = new_node(ND_FOR, NULL, NULL);
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    }
    node->then = stmt();
  } else if (consume("{")) {
    Node head = {};
    Node *cur = &head;
    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    node = new_node(ND_BLOCK, NULL, NULL);
    node->body = head.next;
  } else {
    node = expr();
    expect(";");
  }

  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume("=")) node = new_node(ND_ASSIGN, node, assign());
  return node;
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

Node *unary() {
  if (consume("+")) return primary();
  if (consume("-")) return new_node(ND_SUB, new_node_num(0), unary());
  return primary();
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    if (consume("(")) {
      Node *node = new_node(ND_FUNCALL, NULL, NULL);
      node->funcname = strndup(tok->str, tok->len);
      node->args = func_args();
      return node;
    }

    Var *var = find_var(tok);
    if (!var) {
      var = new_var(strndup(tok->str, tok->len));
    }
    return new_node_var(var);
  }

  return new_node_num(expect_number());
}