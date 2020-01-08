#include "9cc.h"

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
