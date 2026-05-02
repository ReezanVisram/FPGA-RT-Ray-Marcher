#include <verilated.h>

#include "Vtop.h"

int main(int argc, char** argv) {
  VerilatedContext* const contextp = new VerilatedContext;

  contextp->commandArgs(argc, argv);

  Vtop* const top = new Vtop{contextp};

  while (!contextp->gotFinish()) {
    top->eval();
  }

  top->final();

  delete top;

  return 0;
}
