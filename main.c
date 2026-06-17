#include "cpu.h"
#include <stdio.h>

int main(void) {
    static struct CPU cpu; // make static to allow for memory
    reset(&cpu); // initialize a new cpu
    cpu.registers[1] = 5; // value 1 in addr 1
    cpu.registers[2] = 3; // value 2 in addr 2
    word_write(&cpu, 100, 42);
    cpu.registers[1] = 100;
    uint32_t inst = 0x03 | (3 << 7) | (2 << 12) | (1 << 15);  // lw x3, 0(x1)
    word_write(&cpu, 0, inst);
    step(&cpu);
    printf("%u",cpu.registers[3]);
}