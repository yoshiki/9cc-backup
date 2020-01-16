#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: Invalid number of arguments\n", argv[0]);

  // Get user input
  user_input = argv[1];

  // Tokenize
  tokenize();

  // Parse
  Function *prog = program();

  // Count up stack size from locals
  int offset = 0;
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }
  prog->stack_size = offset;

  // Dump node
  if (DEBUG) {
    for (Node *node = prog->node; node; node = node->next) {
      dump_node(node, 0);
    }
    return 0;
  }

  // Output code
  codegen(prog);

  return 0;
}
