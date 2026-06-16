#include "cpu.h"
#include <stdio.h>

int main(void) {
    static struct CPU cpu; // make static to allow for memory
    reset(&cpu); // initialize a new cpu
    cpu.registers[1] = 1; // value 1 in addr 1
    cpu.registers[2] = 2; // value 2 in addr 2
    uint32_t instruction = 0x33 | (3 << 7) | (1 << 15) | (2 << 20);
    word_write(&cpu, 0, instruction);
    step(&cpu);
    printf("%u\n", cpu.registers[3]);
    return 0;
}