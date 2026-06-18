#include <stdint.h>
#define mem_size (1024 * 64) // 64KB allocation
// CPU structure: 32 registers, pc starting @ 0, 1MB memory
struct CPU {
    uint32_t registers[32];
    uint32_t pc;
    uint8_t memory[mem_size];
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