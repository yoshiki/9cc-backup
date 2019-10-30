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

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    
    error_at(p, "Cannot tokenize");
  }
  
  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    return 1;
  }

  // Get user input
  user_input = argv[1];

  // Do tokenize
  currentToken = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Check whether first string is number and then output 'mov' instruction.
  printf("  mov rax, %d\n", expect_number());

  // While doing to spend '+ <num>' or '- num' from token sequence,
  // and output assembly.
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
