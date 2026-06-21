#include "cpu.h"
#include <stdio.h>

int main(void) {
    static struct CPU cpu;
    reset(&cpu);

    loadfile(&cpu, "tests/bins/multiply.bin");
    run(&cpu);
    regview(&cpu);
    return 0;
}