#include "9cc.h"

// Print warn
void warn(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

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

// Print error function
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
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

  switch (node->kind) {
    case ND_NUM:
      printf("%d\n", node->val);
      return;
    case ND_VAR:
      printf("%s(offset: %d)\n", node->var->name, node->var->offset);
      return;
    case ND_ASSIGN:
      printf("=\n");
      dump_node(node->lhs, ++depth);
      dump_node(node->rhs, depth);
      return;
    case ND_RETURN:
      printf("return\n");
      dump_node(node->lhs, ++depth);
      return;
    case ND_IF:
      printf("if ");
      dump_node(node->cond, depth);
      printf("{\n");
      depth++;
      dump_node(node->then, depth);
      printf("}\n");
      if (node->els) {
        printf("else {\n");
        dump_node(node->els, depth);
        printf("}\n");
      }
      return;
    case ND_WHILE:
      printf("while\n");
      dump_node(node->cond, depth);
      printf("{\n");
      dump_node(node->then, depth);
      printf("}\n");
      return;
    case ND_FOR:
      printf("for {\n");
      depth++;
      printf("init:\n");
      dump_node(node->init, depth);
      printf("cond:\n");
      dump_node(node->cond, depth);
      printf("{\n");
      dump_node(node->then, depth);
      printf("}\n");
      printf("inc:\n");
      dump_node(node->inc, depth);
      printf("}\n");
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