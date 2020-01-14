#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0

void warn(char *fmt, ...);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

typedef struct LVar LVar;
struct LVar {
  LVar *next;  // Next variable or NULL
  char *name;  // Variable name
  int len;     // Variable length
  int offset;  // Variable offset from RBP
};

// Local variables
LVar *locals;

// Kind of node for abstract syntax tree
typedef enum {
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_EQ,      // ==
  ND_NE,      // !=
  ND_LT,      // <
  ND_LE,      // <=
  //ND_GT,      // > is invert to ND_LT
  //ND_GE,      // >= is invert to ND_LE
  ND_ASSIGN,  // a = 0
  ND_LVAR,    // Local value
  ND_NUM,     // Integer
  ND_RETURN,  // Return
  ND_IF,      // If
  ND_WHILE,   // While
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;  // Type of node
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  int val;        // Use only the kind is ND_NUM
  int offset;     // Use only the kind is ND_LVAR
  Node *cond;     // If condition
  Node *then;     // If statement
  Node *els;      // Else statement
};

Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *assign();
Node *expr();
Node *stmt();
void program();

void dump_node(Node *node, int depth);

// Kind of token
typedef enum {
  TK_RESERVED, // Symbols
  TK_INDENT,   // Identifier
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

// Input program
char *user_input;

// Fragment of code
Node *code[100];

void gen(Node *node);
void tokenize();
