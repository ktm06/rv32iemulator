#include <stdint.h>
#include <stdio.h>
#define mem_size (1024 * 64) // 64KB allocation
#define debug 1
// CPU structure: 32 registers, pc starting @ 0, 64KB memory
struct CPU {
    uint32_t registers[32];
    uint32_t pc;
    uint8_t memory[mem_size];
    uint8_t stop;
    uint32_t csr[4096];
};

void reset(struct CPU *cpu);
uint8_t byte_read(struct CPU *cpu, uint32_t addr);
void byte_write(struct CPU *cpu, uint32_t addr, uint8_t value);
uint32_t word_read(struct CPU *cpu, uint32_t addr);
void word_write(struct CPU *cpu, uint32_t addr, uint32_t value);
void step(struct CPU *cpu);

void exec_r(struct CPU *cpu, uint32_t instruction);
void exec_i(struct CPU *cpu, uint32_t instruction);
void exec_l(struct CPU *cpu, uint32_t instruction);
void exec_s(struct CPU *cpu, uint32_t instruction);
void exec_b(struct CPU *cpu, uint32_t instruction);
void exec_jalr(struct CPU *cpu, uint32_t instruction);
void exec_jal(struct CPU * cpu, uint32_t instruction);
void exec_lui(struct CPU * cpu, uint32_t instruction);
void exec_auipc(struct CPU *cpu, uint32_t instruction);
void exec_sys(struct CPU * cpu, uint32_t instruction);

void run(struct CPU * cpu);
void regview(struct CPU * cpu);
void loadinstr(struct CPU * cpu, uint32_t *instr);
void loadfile(struct CPU * cpu, char * filename);
