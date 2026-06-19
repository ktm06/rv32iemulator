#include "cpu.h"
#include <stdio.h>

int main(void) {
    static struct CPU cpu; // make static to allow for memory
    reset(&cpu);
    cpu.registers[5] = 0xFF;
    uint32_t inst = 0x73 | (3 << 7) | (1 << 12) | (5 << 15) | (0x340 << 20);  // csrrw x3, 0x340, x5
    word_write(&cpu, 0, inst);
    step(&cpu);
    printf("csr=%X\n", cpu.csr[0x340]);
    printf("x3=%X\n", cpu.registers[3]);;
}