**Overview**:

This is a RISC-V CPU emulator containing the base RV32IM + Zicsr extension instructions. Currently, it runs both hand written programs and compiled C binaries. 

**Architecture**:

The emulator replicates a CPU by executing insturctions through the fetch-decode-exeucte loop:

1. Fetch the instruction located at program counter's address in memory

2. Decode the instruction via opcode and determine operation

3. Execute the intruction, and program counter points to the next instruction.

The CPU has 32 general-purpose 32-bit registers. We follow RISC-V convention by hardwiring x0 to 0 and setting x1 to the stack pointer. x10 is used as the main() function return value. We also have a separate, CSR bank, used by our Ziscr extension. 

RISC-V is a Von Neumann architecture, so our instruction and data live in the same memory. Our emulator is 64KB of memory.

The emulator replicates a RISC-V CPU behaviorally. It is not pipelined or cycle-accurate. 

**Quick Start**:

To compile C code to .bin file: 
1. Have RISC-V GNU Compiler Toolchain (on linux/wsl:  ```sudo apt install gcc-riscv64-unknown-elf```)
2. Create a ```filename.c``` file in src

Run: 
```
riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -nostdlib  -o tests/bins/filename.elf startend.s tests/src/filename.c

riscv64-unknown-elf-objcopy -O binary tests/bins/filename.elf tests/bins/filename.bin
```



**Example**:

We have a minimal C program in ```test.c```: 

``` int main() { return 1; }```

and its binary counterpart is in ```test.bin```. Running this binary through our emulator outputs the following register states:

x0=0 x1=4 x2=65532 .... **x10=1** ... x29=0 x30=0 x31=0

In RISC-V architecture, register x10 is used as to hold the primary function return value (aka main). Since out program simply returns integer 1, our program successfully executed the binary counterpart of ```test.c```.

**Resources used:**

https://fraserinnovations.com/risc-v/risc-v-instruction-set-explanation/

https://msyksphinz-self.github.io/riscv-isadoc/html/rvi.html
