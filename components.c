#include "cpu.h"

void reset(struct CPU *cpu) {
    // 0 initialize struct
    *cpu = (struct CPU) {0};
    cpu->registers[2] = mem_size - 4; //sp
}

uint8_t byte_read(struct CPU *cpu, uint32_t addr) {
    if (addr < mem_size) {
        return cpu->memory[addr];
    }
    // fail condition
    return 0;
}

void byte_write(struct CPU *cpu, uint32_t addr, uint8_t value) {
    if (addr < mem_size) {
        cpu->memory[addr] = value;
    }
}

// 4 byte per word in little endian style

uint32_t word_read(struct CPU *cpu, uint32_t addr) {
    uint32_t word = 0;
    // bit shift & OR
    if (addr + 3 < mem_size) {
        // prevent overflow
        word |= (uint32_t)byte_read(cpu, addr+3) << 24;
        word |= (uint32_t)byte_read(cpu, addr+2) << 16;
        word |= (uint32_t)byte_read(cpu, addr+1) << 8;
        word |= (uint32_t)byte_read(cpu, addr);
    }
    return word;
}

void word_write(struct CPU *cpu, uint32_t addr, uint32_t value) {
    if (addr + 3 < mem_size) {
        // mask with last 8 bits
        byte_write(cpu, addr, value & 0xFF);
        byte_write(cpu, addr+1, (value >> 8) & 0xFF);
        byte_write(cpu, addr+2, (value >> 16) & 0xFF);
        byte_write(cpu, addr+3, (value >> 24) & 0xFF);
    }
}

// cpu fetch execute cycle


void step(struct CPU *cpu) {
    uint32_t instruction = word_read(cpu, cpu->pc);
    cpu->pc += 4;
    // mask to extract opcode
    uint32_t opcode = instruction & 0x7F;
    switch (opcode) {
        case 0b1100111: //JALR
            exec_jalr(cpu, instruction);
            break;
        case 0b0000011: //LOAD
            exec_l(cpu, instruction);
            break; 
        case 0b0010011: //ITYPE
            exec_i(cpu, instruction);
            break;
        case 0b0100011: //STYPE
            exec_s(cpu, instruction);
            break;
        case 0b1100011: //SBTYPE
            exec_b(cpu, instruction);
            break;
        case 0b0110111: //UTYPE1/LUI
            exec_lui(cpu, instruction);
            break;
        case 0b0010111: // UTYPE2/AUIPC
            exec_auipc(cpu, instruction);
            break;
        case 0b1101111: //JAL
            exec_jal(cpu, instruction);
            break;
        case 0b0110011: //RTYPE
            exec_r(cpu, instruction);
            break;
        case 0b1110011: // SYSTEM
            exec_sys(cpu, instruction);
        case 0b1111111: // sentinel case
            if (debug) printf("sentinel\n");
            cpu->stop = 0x1;
            break;
        default:
            if (debug) printf("bad instruction, general\n");
            cpu->stop = 0x1;
            break;
    }
    cpu->registers[0] = 0; // risc rule of 0 @ 0
}

void run(struct CPU *cpu) {
    puts("Begin");
    int i= 1;
    while (cpu->stop == 0x0) {
        printf("Instruction %i:\n", i);
        step(cpu);
        i++;
        if (i > 10000) {
            break;
        }
    }
}

void regview(struct CPU *cpu) {
    printf("pc=%u\n", cpu->pc);
    for (int i = 0; i<32;i++) {
        printf("x%i=%u ", i, cpu->registers[i]);
    }
}

void loadinstr(struct CPU *cpu,uint32_t *instr) {
    int i =0;
    for (const uint32_t *ptr = instr; *ptr != 0xFFFFFFFF; ptr++) { // sentinel
        word_write(cpu, i, *ptr);
        i += 4;
    }
}

void loadfile(struct CPU *cpu, char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f==NULL) {
        puts("Failed to read file");
        return;
    }
    fread(cpu->memory, 1, mem_size, f);
    fclose(f);
}
// execute functions

void exec_r(struct CPU *cpu, uint32_t instruction) {
    // bit shifting & mask
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t funct3 = instruction >> 12 & 0x07;
    uint32_t rs1_addr= instruction >> 15 & 0x1F;
    uint32_t rs2_addr = instruction >> 20 & 0x1F;
    uint32_t funct7 = instruction >> 25 & 0x7F;

    uint32_t rs1 = cpu->registers[rs1_addr];
    uint32_t rs2 = cpu->registers[rs2_addr];
    if (funct7 == 0) {
    switch (funct3) {
        case (0x0): // add or sub
        if (funct7 == 0) {
            if (debug) printf("ADD: x%u = %u@x%u + %u@x%u\n", rd, rs1, rs1_addr, rs2, rs2_addr);
            cpu->registers[rd] = rs1 + rs2;
        } else {
            if (debug) printf("SUB: x%u = %u@x%u - %u@x%u\n", rd,rs1, rs1_addr, rs2, rs2_addr);
            cpu->registers[rd] = rs1 -rs2;
        }
            break;
        case (0x1): // sll logical shift left
            if (debug) printf("SLL: x%u = %u@x%u << %u@x%u\n", rd,rs1, rs1_addr, rs2, rs2_addr);
            cpu->registers[rd] = rs1 << (rs2 & 0x1F);
            break;
        case (0x2): // slt set less than if rs1 < rs2 place 1 in addr else 0
        // op needs us to turn it into a signed
            if (debug) printf("SLT: x%u = %u@x%u < (s) %u@x%u\n", rd,rs1, rs1_addr, rs2, rs2_addr);
            if ((int32_t)rs1 < (int32_t)rs2) {
                cpu->registers[rd] = 1;
            } else {
                cpu->registers[rd] = 0;
            }
            break;
        case (0x3): // sltu same as above but for unsigned
            if (debug) printf("SLTU: x%u = %u@x%u < (u) %u@x%u\n", rd,rs1, rs1_addr, rs2, rs2_addr);
            if (rs1 < rs2) {
                
                cpu->registers[rd] = 1;
            } else {
                cpu->registers[rd] = 0;
            }
            break;
        case (0x4): // xor
            if (debug) printf("XOR: x%u = %u@x%u XOR %u@x%u\n", rd,rs1, rs1_addr, rs2, rs2_addr);
            cpu->registers[rd] = rs1 ^ rs2;
            break;
        case (0x5): // srl logical shift right or arithmatic shift right
            if (funct7 == 0) { 
                if (debug) printf("SRL: x%u = %u@x%u >> %u@x%u\n", rd, rs1, rs1_addr, rs2, rs2_addr);
                cpu->registers[rd] = rs1 >> (rs2 & 0x1F); //srl
            } else {
                if (debug) printf("SRA: x%u = %u@x%u >> (s) %u@x%u\n", rd, rs1, rs1_addr, rs2, rs2_addr);
                cpu->registers[rd] = (int32_t) rs1 >> (rs2 & 0x1F);// sra
            }
            break;
        case (0x6): // or
            if (debug) printf("OR: x%u = %u@x%u OR %u@x%u\n", rd,rs1, rs1_addr, rs2, rs2_addr);
            cpu->registers[rd] = rs1 | rs2;
            break;
        case (0x7): // and
            if (debug) printf("AND: x%u = %u@x%u AND %u@x%u\n", rd,rs1, rs1_addr, rs2, rs2_addr);
            cpu->registers[rd] = rs1 & rs2;
            break;
    }
} else if (funct7 == (0x1)) {
    //M extension
    switch (funct3) {
        case (0x0): //mul
            cpu->registers[rd] = rs1 * rs2;
            break;
        case (0x1): {//mulh
            int64_t temp = (int64_t)(int32_t)rs1 * (int64_t)(int32_t)rs2;
            cpu->registers[rd] = (uint32_t) (temp >> 32);
            break;
        }
        case (0x2): {//mulhsu
            int64_t temp = (int64_t)(int32_t)rs1 * (uint64_t)rs2;
            cpu->registers[rd] = (uint32_t) (temp >> 32);
            break;
        }
        case (0x3):{ //mulhu
            uint64_t temp = (uint64_t)rs1 * (uint64_t)rs2;
            cpu->registers[rd] = (uint32_t) (temp >> 32);
            break;
        }
        case (0x4): //div
            if (rs2 == 0) {
                cpu->registers[rd] = 0xFFFFFFFF;
            } else if (rs2 == 0xFFFFFFFF && rs1 == 0x80000000){
                cpu->registers[rd] = 0x80000000;
            } else {
                cpu->registers[rd] = (int32_t)rs1 / (int32_t)rs2;
            }
            break;
        case (0x5): //divu
        if (rs2 == 0){
            cpu->registers[rd] = 0xFFFFFFFF;
        } else {
            cpu->registers[rd] = rs1 / rs2;
        }
            break;
        case (0x6): //rem
        if (rs2 == 0) {
                cpu->registers[rd] = rs1;
            } else if (rs2 == 0xFFFFFFFF && rs1 == 0x80000000){
                cpu->registers[rd] = 0;
            } else {
                cpu->registers[rd] = (int32_t)rs1 % (int32_t)rs2;
            }
            break;
        case (0x7): //remu
            if (rs2 == 0){
            cpu->registers[rd] = rs1;
        } else {
            cpu->registers[rd] = rs1 % rs2;
        }
            break;
    }
} else {
    if (debug) printf("bad instruction, R-type\n");
    cpu->stop = 0x1;
}

}

void exec_i(struct CPU *cpu, uint32_t instruction) {
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t funct3 = instruction >> 12 & 0x7;
    uint32_t rs1_addr = instruction >> 15 & 0x1F;
    uint32_t imm = instruction >> 20 & 0xFFF;

    // sign extensions i.e. if msb is 1, make all left 1, else make all left 0
    if ((imm & 0x800) == 0x800) {
        imm |= 0xFFFFF000;
    } else {
        imm &= 0x00000FFF;
    }
    uint32_t rs1 = cpu->registers[rs1_addr];
    switch (funct3) {
        case (0x0): // addi
        if (debug) printf("ADDI: x%u = %u@x%u + %u\n", rd,rs1, rs1_addr, imm);
        cpu->registers[rd] = imm + rs1;
            break;
        case (0x1): // slli
        if (debug) printf("SLLI: x%u = %u@x%u << %u\n", rd,rs1, rs1_addr, imm);
        cpu->registers[rd] = rs1 << (imm & 0x1F);
            break; 
        case (0x2): // slti
        if (debug) printf("SLTI: x%u = %u@x%u < (s) %u\n", rd,rs1, rs1_addr, imm);
        if ((int32_t) rs1 < (int32_t) imm) {
            cpu->registers[rd] = 1;
        } else {
            cpu->registers[rd] = 0;
        }
            break;
        case (0x3): // sltiu
        if (debug) printf("SLTIU: x%u = %u@x%u < (u) %u\n", rd,rs1, rs1_addr, imm);
        if (rs1 < imm) {
            cpu->registers[rd] = 1;
        } else {
            cpu->registers[rd] = 0;
        }
            break;
        case (0x4): // xori
            if (debug) printf("XORI: x%u = %u@x%u XOR %u\n", rd,rs1, rs1_addr, imm);
            cpu->registers[rd] = rs1 ^ imm;
            break;
        case (0x5): // srli or srai
            if ((imm >> 5) == 0) {
                if (debug) printf("SRLI: x%u = %u@x%u >> (u) %u\n", rd,rs1, rs1_addr, imm);
                cpu->registers[rd] = rs1 >> (imm & 0x1F);
            } else {
                if (debug) printf("SLTI: x%u = %u@x%u >> (s) %u\n", rd,rs1, rs1_addr, imm);
                cpu->registers[rd] = (int32_t) rs1 >> (imm & 0x1F);
            }
            break;
        case (0x6): // ori
        if (debug) printf("ORI: x%u = %u@x%u OR %u\n", rd,rs1, rs1_addr, imm);
        cpu->registers[rd] = imm | rs1;
            break;
        case (0x7): // andi
        if (debug) printf("SLTI: x%u = %u@x%u AND %u\n", rd,rs1, rs1_addr, imm);
        cpu->registers[rd] = imm & rs1;
            break;
    }

}

void exec_l(struct CPU *cpu, uint32_t instruction) {
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t funct3 = instruction >> 12 & 0x7;
    uint32_t rs1_addr = instruction >> 15 & 0x1F;
    uint32_t offset = instruction >> 20 & 0xFFF;   
    if ((offset & 0x800) == 0x800) {
        offset |= 0xFFFFF000;
    } else {
        offset &= 0x00000FFF;
    }
    uint32_t rs1 = cpu->registers[rs1_addr];

    switch (funct3) {
        case (0x0): // lb 8bit load w sign extend, "sext" means sign extend
            if (debug) printf("LB: x%u = sext(M[%u@x%u + %u[7:0]])\n", rd,rs1, rs1_addr, offset);
            cpu->registers[rd] = (int32_t)(int8_t)byte_read(cpu,(offset + rs1));
            break;
        case (0x1): // lh 16bit load w sign etend
        if (debug) printf("LH: x%u = sext(M[%u@x%u + %u[15:0]])\n", rd,rs1, rs1_addr, offset);
            cpu->registers[rd] = (int32_t)(int16_t)word_read(cpu, (offset + rs1));
            break;
        case (0x2): // lw 32bit load
        if (debug) printf("LW: x%u = sext(M[%u@x%u + %u[31:0]])\n", rd,rs1, rs1_addr, offset);
            cpu->registers[rd] = word_read(cpu, (offset  + rs1));
            break;
        case (0x4): // lbu 8 bit load w 0 extend 
        if (debug) printf("LBU: x%u = M[%u@x%u + %u[7:0]]\n", rd,rs1, rs1_addr, offset);
            cpu->registers[rd] = byte_read(cpu, ((offset + rs1))); 
            break;
        case (0x5): // lhu 16 bit load w 0 extend
            if (debug) printf("LHU: x%u = M[%u@x%u + %u[15:0]]\n", rd,rs1, rs1_addr, offset);
            cpu->registers[rd] = word_read(cpu, (offset + rs1)) & 0xFFFF;
            break;
        default:
            if (debug) printf("bad instruction, L-type\n");
            cpu->stop = 0x1;
            break;
    }


}

void exec_s(struct CPU *cpu, uint32_t instruction) {
    uint32_t offset5 = instruction >> 7 & 0x1F;
    uint32_t funct3 = instruction >> 12 & 0x7;
    uint32_t rs1_addr = instruction >> 15 & 0x1F;
    uint32_t rs2_addr = instruction >> 20 & 0x1F;
    uint32_t offset7 = instruction >> 25 & 0x7F;

    // combine offset
    uint32_t offset = offset5 | (offset7 << 5);

    if ((offset & 0x800) == 0x800) {
        offset |= 0xFFFFF000;
    } else {
        offset &= 0x00000FFF;
    }

    switch (funct3) {
        case (0x0): // sb
            if (debug) printf("SB: M[%u@x%u + %u] = %u@%u[7:0]\n", cpu->registers[rs1_addr], rs1_addr, offset, cpu->registers[rs2_addr], rs2_addr);
            byte_write(cpu, (cpu->registers[rs1_addr] + offset), (cpu->registers[rs2_addr] & 0xFF));
            break;
        case (0x1): // sh
            if (debug) printf("SH: M[%u@x%u + %u] = %u@%u[15:0]\n", cpu->registers[rs1_addr], rs1_addr, offset, cpu->registers[rs2_addr], rs2_addr);
            byte_write(cpu, (cpu->registers[rs1_addr] + offset), ((cpu->registers[rs2_addr] & 0xFF)));
            byte_write(cpu, (cpu->registers[rs1_addr] + offset + 1), ((cpu->registers[rs2_addr] >> 8) & 0xFF));
            break;
        case (0x2): // sw
            if (debug) printf("SW: M[%u@x%u + %u] = %u@%u[31:0]\n", cpu->registers[rs1_addr], rs1_addr, offset, cpu->registers[rs2_addr], rs2_addr);
            word_write(cpu, (cpu->registers[rs1_addr] + offset), ((cpu->registers[rs2_addr] & 0xFFFFFFFF)));
            break;
        default:
            if (debug) printf("bad instruction, S-type\n");
            cpu->stop = 0x1;
            break;
    }
}

void exec_b(struct CPU *cpu, uint32_t instruction) {
    // immediate is not ordered so decode using instr format
    uint32_t funct3 = instruction >> 12 & 0x7;
    uint32_t rs1_addr = instruction >> 15 & 0x1F;
    uint32_t rs2_addr = instruction >> 20 & 0x1F;
    uint32_t offset4_1 = instruction >> 8 & 0xF;
    uint32_t offset10_5 = instruction >> 25 & 0x3F;
    uint32_t offset11 = instruction >> 7 & 0x1;
    uint32_t offset12 = instruction >> 31 & 0x1;
    uint32_t rs1 = cpu->registers[rs1_addr];
    uint32_t rs2 = cpu->registers[rs2_addr];
    // rebuild offset
    uint32_t offset = offset4_1 << 1| offset10_5 << 5 | offset11 << 11 | offset12 << 12;
    
    if ((offset & 0x1000) == 0x1000) {
        offset |= 0xFFFFE000;
    } else {
        offset &= 0x00001FFF;
    }

    switch (funct3) {
        case (0x0): // beq
            if (rs1 == rs2) {
                if (debug) printf("BEQ: if (%u@x%u == %u@x%u) pc += %u\n", rs1, rs1_addr, rs2, rs2_addr, offset);
                cpu->pc -= 4; // since we increment in step we must deincrement
                cpu->pc += offset;
            }
            break;
        case (0x1): // bne
            if (debug) printf("BNE: if (%u@x%u != %u@x%u) pc += %u\n", rs1, rs1_addr, rs2, rs2_addr, offset);
            if (rs1 != rs2) {
                    cpu->pc -= 4; 
                    cpu->pc += offset;
            }
            break;
        case (0x4): // blt
            if (debug) printf("BLT: if (%u@x%u < %u@x%u) pc += %u\n", rs1, rs1_addr, rs2, rs2_addr, offset);
            if ((int32_t) rs1 < (int32_t) rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        case (0x5): // bge
            if (debug) printf("BGE: if (%u@x%u <= %u@x%u) pc += %u\n", rs1, rs1_addr, rs2, rs2_addr, offset);
            if ((int32_t) rs1 >= (int32_t) rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        case (0x6): // bltu
            if (debug) printf("BLTU: if (%u@x%u < (u) %u@x%u) pc += %u\n", rs1, rs1_addr, rs2, rs2_addr, offset);
            if (rs1 < rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        case (0x7): // bgeu
            if (debug) printf("BGEU: if (%u@x%u >= (u) %u@x%u) pc += %u\n", rs1, rs1_addr, rs2, rs2_addr, offset);
            if (rs1 >= rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        default:
            if (debug) printf("bad instruction, B-type\n");
            cpu->stop = 0x1;
            break;
    }
}

void exec_jalr(struct CPU *cpu, uint32_t instruction) {
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t rs1_addr = instruction >> 15 & 0x1F;
    uint32_t offset = instruction >> 20 & 0xFFF;
    // sign extend
    if ((offset & 0x800) == 0x800) {
        offset |= 0xFFFFF000;
    } else {
        offset &= 0x00000FFF;
    }
    if (debug) printf("JALR: x%u = pc+4, pc = (%u@x%u + %u)&~1\n", rd, cpu->registers[rs1_addr], rs1_addr, offset);
    
    uint32_t temp=cpu->pc;
    
    cpu->pc = (cpu->registers[rs1_addr] + offset) & ~1; // force even by clearing lowest bit to 0
    cpu->registers[rd] = temp;
}

void exec_jal(struct CPU *cpu, uint32_t instruction) {
    uint32_t rd = instruction >> 7 & 0x1F;
    
    // like b-type, we need to rebuild offset 
    uint32_t offset10_1 = instruction >> 21 & 0x3FF;
    uint32_t offset11 = instruction >> 20 & 0x1;
    uint32_t offset19_12 = instruction >> 12 & 0xFF;
    uint32_t offset20 = instruction >> 31 & 0x1;
    uint32_t offset = offset10_1 << 1 | offset11 << 11 | offset19_12 << 12 | offset20 << 20;
    // sign extend
    if ((offset & 0x100000) == 0x100000) {
        offset |= 0xFFE00000;
    } else {
        offset &= 0x001FFFFF;
    }
    if (debug) printf("JAL: x%u = pc+4, pc += %u\n", rd, offset);
    cpu->registers[rd] = cpu->pc;
    cpu->pc -= 4; // decrement for the 4 we move up
    cpu->pc += offset;
}

void exec_lui(struct CPU *cpu, uint32_t instruction) {
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t imm = instruction >> 12 & 0xFFFFF;
    if ((imm & 0x80000) == 0x80000) {
        imm |= 0xFFF00000;
    } else {
        imm &= 0x002FFFFF;
    }
    if (debug) printf("LUI: x%u = %u[32:12] << 12\n", rd, imm);
    cpu->registers[rd] = imm << 12;
}

void exec_auipc(struct CPU *cpu, uint32_t instruction) {
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t imm = instruction >> 12 & 0xFFFF;
    if ((imm & 0x80000) == 0x80000) {
        imm |= 0xFFF00000;
    } else {
        imm &= 0x002FFFFF;
    }
    if (debug) printf("AUIPC: x%u = pc + %u[32:12] << 12\n", rd, imm);
    cpu->registers[rd] = (imm<<12) + cpu->pc -4;
}

void exec_sys(struct CPU *cpu, uint32_t instruction) {
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t funct3 = instruction >> 12 & 0x7;
    uint32_t rs1_addr = instruction >> 15 & 0x1F;
    uint32_t csr_addr = instruction >> 20 & 0xFFF;
    uint32_t rs1 = cpu->registers[rs1_addr];
    //0extend csr addr
    uint32_t temp = cpu->csr[csr_addr];
    switch (funct3) {
        case (0x0):
            switch (csr_addr) {
                case (0): //ecall
                    if (debug) printf("ECALL\n"); 
                    cpu->stop = 0x1;
                    break;
                case (0x1): //ebreak
                if (debug) printf("EBREAK\n"); 
                    cpu->stop = 0x1;
                    break;
                case (0x2): //uret
                if (debug) printf("URET\n"); 
                    cpu->stop = 0x1;
                    break;
                case (0x102): // sret
                if (debug) printf("SRET\n"); 
                    cpu->stop = 0x1;
                    break;
                case(0x302): //mret
                if (debug) printf("MRET\n"); 
                    cpu->stop = 0x1;
                    break;
                case(0x105): //wfi
                if (debug) printf("WFI\n"); 
                    break;
                default: // for now also includes sfence vma
                if (debug) printf("bad instruction, sys type\n"); 
                    cpu->stop = 0x1;
                    break;
            }
            break;
        case (0x1): // csrrw
            if (debug) printf("CSRRW: x%u = CSR[%u]; CSR[%u] = %u@x%u\n", rd, csr_addr, csr_addr,rs1, rs1_addr);
            cpu->csr[csr_addr] = rs1;
            if (rd != 0) {
                cpu->registers[rd] = temp;
            }
            break;
        case(0x2): //csrrs
            if (debug) printf("CSRRS: x%u = CSR[%u]; CSR[%u] = CSR[%u] | %u@x%u\n", rd, csr_addr, csr_addr,csr_addr,rs1, rs1_addr);
            if (rs1_addr!=0) {
                cpu->csr[csr_addr] = temp | rs1;
            }
            cpu->registers[rd] = temp;
            break;
        case(0x3): //csrrc
            if (debug) printf("CSRRC: x%u = CSR[%u]; CSR[%u] = CSR[%u] & ~%u@x%u\n", rd, csr_addr, csr_addr,csr_addr,rs1, rs1_addr);
            if (rs1_addr!=0) {
                cpu->csr[csr_addr] = temp & ~rs1;
            }
            cpu->registers[rd] = temp;
            break;
        case(0x5): //csrrwi
            if (debug) printf("CSRRWI: x%u = CSR[%u]; CSR[%u] = zext[%u]\n", rd, csr_addr, csr_addr,rs1_addr);
            if (rd!= 0) {
                cpu->registers[rd] = temp;
            }
            cpu->csr[csr_addr] = rs1_addr; //uimm 
            break;
        case(0x6): //csrrsi
            if (debug) printf("CSRRSI: x%u = CSR[%u]; CSR[%u] = CSR[%u] | zext[%u]\n", rd, csr_addr, csr_addr,csr_addr, rs1_addr);
            if (rs1_addr!=0) {
                cpu->csr[csr_addr] = temp | rs1_addr; 
            }
            cpu->registers[rd] = temp;
            break;
        case(0x7): //csrrci
            if (debug) printf("CSRRCI: x%u = CSR[%u]; CSR[%u] = CSR[%u] & ~zext[%u]\n", rd, csr_addr, csr_addr,csr_addr, rs1_addr);
            if (rs1_addr!=0) {
                cpu->csr[csr_addr] = temp & ~rs1_addr;
            }
            cpu->registers[rd] = temp;
            break;
        default:
            if (debug) printf("bad instruction, sys type");
            cpu->stop = 0x1;
            break;
        }
    }