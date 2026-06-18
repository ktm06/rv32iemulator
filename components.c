#include "cpu.h"

void reset(struct CPU *cpu) {
    // 0 initialize struct
    *cpu = (struct CPU) {0};
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
        case 0b0110111: //UTYPE1
            break;
        case 0b0010111: //UTYPE2
            break;
        case 0b1101111: //JTYPE
            break;
        case 0b0110011: //RTYPE
            exec_r(cpu, instruction);
            break;
        default:
        break;
    }
}

void exec_r(struct CPU *cpu, uint32_t instruction) {
    // bit shifting & mask
    uint32_t rd = instruction >> 7 & 0x1F;
    uint32_t funct3 = instruction >> 12 & 0x07;
    uint32_t rs1_addr= instruction >> 15 & 0x1F;
    uint32_t rs2_addr = instruction >> 20 & 0x1F;
    uint32_t funct7 = instruction >> 25 & 0x7F;

    uint32_t rs1 = cpu->registers[rs1_addr];
    uint32_t rs2 = cpu->registers[rs2_addr];

    switch (funct3) {
        case (0x0): // add or sub
        if (funct7 == 0) {
            cpu->registers[rd] = rs1 + rs2;
        } else {
            cpu->registers[rd] = rs1 -rs2;
        }
            break;
        case (0x1): // sll logical shift left
            cpu->registers[rd] = rs1 << (rs2 & 0x1F);
            break;
        case (0x2): // slt set less than if rs1 < rs2 place 1 in addr else 0
        // op needs us to turn it into a signed
            if ((int32_t)rs1 < (int32_t)rs2) {
                cpu->registers[rd] = 1;
            } else {
                cpu->registers[rd] = 0;
            }
            break;
        case (0x3): // sltu same as above but for unsigned
            if (rs1 < rs2) {
                cpu->registers[rd] = 1;
            } else {
                cpu->registers[rd] = 0;
            }
            break;
        case (0x4): // xor
            cpu->registers[rd] = rs1 ^ rs2;
            break;
        case (0x5): // srl logical shift right or arithmatic shift right
            if (funct7 == 0) { 
                cpu->registers[rd] = rs1 >> (rs2 & 0x1F); //srl
            } else {
                cpu->registers[rd] = (int32_t) rs1 >> (rs2 & 0x1F);// sra
            }
            break;
        case (0x6): // or
            cpu->registers[rd] = rs1 | rs2;
            break;
        case (0x7): // and
            cpu->registers[rd] = rs1 & rs2;
            break;
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
        cpu->registers[rd] = imm + rs1;
            break;
        case (0x1): // slli
        cpu->registers[rd] = rs1 << (imm & 0x1F);
            break; 
        case (0x2): // slti
        if ((int32_t) rs1 < (int32_t) imm) {
            cpu->registers[rd] = 1;
        } else {
            cpu->registers[rd] = 0;
        }
            break;
        case (0x3): // sltiu
        if (rs1 < imm) {
            cpu->registers[rd] = 1;
        } else {
            cpu->registers[rd] = 0;
        }
            break;
        case (0x4): // xori
            cpu->registers[rd] = rs1 ^ imm;
            break;
        case (0x5): // srli or srai
            if ((imm >> 5) == 0) {
                cpu->registers[rd] = rs1 >> (imm & 0x1F);
            } else {
                cpu->registers[rd] = (int32_t) rs1 >> (imm & 0x1F);
            }
            break;
        case (0x6): // ori
        cpu->registers[rd] = imm | rs1;
            break;
        case (0x7): // andi
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
        case (0x0): // lb 8bit load w sign extend
            cpu->registers[rd] = (int32_t)(int8_t)byte_read(cpu,(offset + rs1));
            break;
        case (0x1): // lh 16bit load w sign etend
            cpu->registers[rd] = (int32_t)(int16_t)word_read(cpu, (offset + rs1));
            break;
        case (0x2): // lw 32bit load
            cpu->registers[rd] = word_read(cpu, (offset  + rs1));
            break;
        case (0x4): // lbu 8 bit load w 0 extend 
            cpu->registers[rd] = byte_read(cpu, ((offset + rs1))); 
            break;
        case (0x5): // lhu 16 bit load w 0 extend
            cpu->registers[rd] = word_read(cpu, (offset + rs1)) & 0xFFFF;
            break;
        default:
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
            byte_write(cpu, (cpu->registers[rs1_addr] + offset), (cpu->registers[rs2_addr] & 0xFF));
            break;
        case (0x1): // sh
            byte_write(cpu, (cpu->registers[rs1_addr] + offset), ((cpu->registers[rs2_addr] & 0xFF)));
            byte_write(cpu, (cpu->registers[rs1_addr] + offset + 1), ((cpu->registers[rs2_addr] >> 8) & 0xFF));
            break;
        case (0x2): // sw
            word_write(cpu, (cpu->registers[rs1_addr] + offset), ((cpu->registers[rs2_addr] & 0xFFFFFFFF)));
            break;
        default:
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
                cpu->pc -= 4; // since we increment in step we must deincrement
                cpu->pc += offset;
            }
            break;
        case (0x1): // bne
            if (rs1 != rs2) {
                    cpu->pc -= 4; 
                    cpu->pc += offset;
            }
            break;
        case (0x4): // blt
            if ((int32_t) rs1 < (int32_t) rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        case (0x5): // bge
            if ((int32_t) rs1 >= (int32_t) rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        case (0x6): // bltu
            if (rs1 < rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        case (0x7): // bgeu
            if (rs1 >= rs2) {
                cpu->pc -= 4; 
                cpu->pc += offset;
            }
            break;
        default:
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
    cpu->registers[rd] = cpu->pc;
    cpu->pc = (cpu->registers[rs1_addr] + offset) & ~1; // force even by clearing lowest bit to 0
}