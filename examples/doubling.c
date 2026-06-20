#include "../cpu.h"
#include <stdio.h>


// this program doubles the 
int main(void) {
    static struct CPU cpu;
    reset(&cpu);

    uint32_t program[] = {   
        0x13 | (2<<7) | (50<<20), //addi, rd =2, rs1=0, imm=1
        0x6F | (3<<7) | (0x8<<20), //jal, rd=3, offset = 8
        0x73, //ecall
        0x33 | (2<<7) | (2<<15)|(2<<20), //addi rd=2, rs1=2 rs2=2
        0x67 | (3<<7)|(3<<15), //jalr rs1=3 rd=3
        0xFFFFFFFF,
    };

    loadinstr(&cpu, program);
    run(&cpu);
    regview(&cpu);
    return 0;
}