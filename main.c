#include "cpu.h"
#include <stdio.h>

int main(void) {
    static struct CPU cpu; // make static to allow for memory
    reset(&cpu);
    cpu.registers[1] = 5;
    cpu.registers[2] = 5;   
    uint32_t inst = 0x63 | (0 << 12) | (1 << 15) | (2 << 20) | (1 << 10);  // beq x1,x2, offset 8
    word_write(&cpu, 0, inst);
    step(&cpu);
    printf("%u\n", cpu.pc);
    reset(&cpu);
    cpu.registers[1] = 5;
    cpu.registers[2] = 6;   
    inst = 0x63 | (0 << 12) | (1 << 15) | (2 << 20) | (1 << 10);
    word_write(&cpu, 0, inst);
    step(&cpu);
    printf("%u\n", cpu.pc);

    return 0;
}