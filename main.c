#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: Invalid number of arguments\n", argv[0]);

  // Get user input
  user_input = argv[1];

  // Tokenize
  tokenize();

  // Parse
  program();

  // Count up stack size from locals
  int stack_size = 0;
  for (LVar *var = locals; var; var = var->next) {
    stack_size += 8;
  }

  // Dump node
  if (DEBUG) {
    for (int i = 0; code[i]; i++) {
      dump_node(code[i], 0);
    }
    return 0;
  }

  // Output header of assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Prologue
  // Allocate space for 26 variables
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", stack_size);

  // Generate code while traversing AST to emit assembly
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // Pop one value because remains on the stack
    // as the result of evaluating the expression
    printf("  pop rax\n");
  }

  // Epilogue
  // The rax that is result of last evaluating become return value
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

  return 0;
}
