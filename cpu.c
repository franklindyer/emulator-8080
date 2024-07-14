#include <stdio.h>


#define DCR(cpu,r) \
    r += -1; \
    (cpu->flags).z = (r == 0); \
    (cpu->flags).s = r >> 7; \
    SETPARITY((cpu->flags).p, r) \
    (cpu->flags).ac = (r & 0xf) == 0xf;
#define SUB(cpu,a,r) \
    (cpu->flags).z = (a == r); \
    (cpu->flags).c = (a < r); \
    a = a - r; \
    (cpu->flags).s = (a & 0x80) >> 7; \
    SETPARITY((cpu->flags).p, a);
#define SETPARITY(p,x) p=x; p=((p)>>4)^((p)&0xf); p=((p)>>2)^((p)&3); p=((p)>>1)^((p)&1);    

typedef struct flags8080 {
    uint8_t z : 1;
    uint8_t s : 1;
    uint8_t p : 1;
    uint8_t c : 1;
    uint8_t ac : 1;
    uint8_t ei : 1;
    uint8_t padding : 2;
} flags8080;

typedef struct cpu8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t pc;
    uint16_t sp;
    flags8080 flags;
    uint8_t* memory;
} cpu8080;

void print_cpu_state(cpu8080* cpu) {
    printf("REGISTERS:\n");
    printf("\tA\t0x%02x\n", cpu->a);
    printf("\tB\t0x%02x\n", cpu->b);
    printf("\tC\t0x%02x\n", cpu->c);
    printf("\tD\t0x%02x\n", cpu->d);
    printf("\tE\t0x%02x\n", cpu->e);
    printf("\tH\t0x%02x\n", cpu->h);
    printf("\tL\t0x%02x\n", cpu->l);
    printf("\tPC\t0x%04x\n", cpu->pc);
    printf("\tSP\t0x%04x\n", cpu->sp);
    printf("FLAGS: ");
    if ((cpu->flags).z) printf("zero ");
    if ((cpu->flags).s) printf("sign ");
    if ((cpu->flags).p) printf("parity ");
    if ((cpu->flags).c) printf("carry ");
    if ((cpu->flags).ac) printf("auxcarry ");
    printf("\n");
}

void unimplemented_op(cpu8080* cpu, unsigned char opcode) {
    printf("Unimplemented operation: 0x%02x\nCPU STATE:\n", opcode);
    print_cpu_state(cpu);
    exit(1);
}

void emulate_cpu8080(cpu8080* cpu, long bound) {
    unsigned char *op;
    unsigned char *mem = cpu->memory;
    uint16_t pc;
    uint16_t aux;
    while (bound > 0) {
        bound--;
        pc = cpu->pc;
        op = &cpu->memory[cpu->pc];
        printf("At 0x%04x doing operation 0x%02x\n", cpu->pc, *op);
        cpu->pc++;
        switch(*op) {

            case 0x00: // NOP
                break;

            case 0x05: // DCR B
                DCR(cpu,cpu->b)
                break;

            case 0x06: // MVI B D8
                cpu->b = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x0d: // DCR C
                DCR(cpu,cpu->c)
                break;

            case 0x0e: // MVI C D8
                cpu->c = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x11: // LXI DE D16
                cpu->d = mem[pc+2];
                cpu->e = mem[pc+1];
                cpu->pc += 2;
                break;

            case 0x13: // INX DE
                aux = (cpu->d << 8) + cpu->e;
                aux++;
                cpu->d = aux >> 8;
                cpu->e = aux & 255;
                break;

            case 0x15: // DCR D
                DCR(cpu,cpu->d)
                break;

            case 0x16: // MVI D D8
                cpu->d = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x1a: // LDAX DE
                aux = (cpu->d << 8) + cpu->e;
                cpu->a = mem[aux];
                break;

            case 0x1d: // DCR E
                DCR(cpu,cpu->e)
                break;

            case 0x1e: // MVI E D8
                cpu->e = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x21: // LXI HL D16
                cpu->h = mem[pc+2];
                cpu->l = mem[pc+1];
                cpu->pc += 2;
                break;

            case 0x23: // INX HL
                aux = (cpu->h << 8) + cpu->l;
                aux++;
                cpu->h = aux >> 8;
                cpu->l = aux & 255;
                break;

            case 0x25: // DCR H
                DCR(cpu,cpu->h)
                break;

            case 0x26: // MVI H D8
                cpu->h = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x2d: // DCR L
                DCR(cpu,cpu->l)
                break;

            case 0x2e: // MVI L D8
                cpu->l = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x31: // LXI SP D16
                cpu->sp = (mem[pc+2] << 8) + mem[pc+1];
                cpu->pc += 2;
                break;

            case 0x35: // DCR M
                aux = (cpu->h << 8) + cpu->l;
                DCR(cpu,mem[aux])
                break;

            case 0x36: // MVI M D8
                aux = (cpu->h << 8) + cpu->l;
                mem[aux] = mem[pc+1];
                cpu->pc++;
                break;

            case 0x3d: // DCR A
                DCR(cpu,cpu->a)
                break;

            case 0x3e: // MVI A D8
                cpu->a = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x56: // MOV D M
                aux = (cpu->h << 8) + cpu->l;
                cpu->d = mem[aux];
                break;

            case 0x5e: // MOV E M
                aux = (cpu->h << 8) + cpu->l;
                cpu->e = mem[aux];
                break;

            case 0x66: // MOV H M
                aux = (cpu->h << 8) + cpu->l;
                cpu->h = mem[aux];
                break;

            case 0x6f: // MOV L A
                cpu->l = cpu->a;
                break;

            case 0x77: // MOV M A
                aux = (cpu->h << 8) + cpu->l;
                mem[aux] = cpu->a;
                break;

            case 0x7a: // MOV A D
                cpu->a = cpu->d;
                break;

            case 0x7b: // MOV A E
                cpu->a = cpu->e;
                break;

            case 0x7c: // MOV A H
                cpu->a = cpu->h;
                break;

            case 0x7e: // MOV A M
                aux = (cpu->h << 8) + cpu->l;
                cpu->a = mem[aux];
                break;

            case 0xc2: // JNZ D16
                if (!(cpu->flags).z) cpu->pc = (mem[pc+2] << 8) + mem[pc+1];
                else cpu->pc += 2;
                break; 

            case 0xc3: // JMP D16
                cpu->pc = (mem[pc+2] << 8) + mem[pc+1];
                break;

            case 0xc5: // PUSH BC
                mem[cpu->sp-1] = cpu->b;
                mem[cpu->sp-2] = cpu->c;
                cpu->sp += -2;
                break;

            case 0xc9: // RET
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xcd: // CALL
                mem[cpu->sp-1] = (pc+3) >> 8;
                mem[cpu->sp-2] = (pc+3) & 0xff;
                cpu->sp += -2;
                cpu->pc = (mem[pc+2] << 8) + mem[pc+1];
                break;

            case 0xd5: // PUSH DE
                mem[cpu->sp-1] = cpu->d;
                mem[cpu->sp-2] = cpu->e;
                cpu->sp += -2;
                break;

            case 0xe5: // PUSH HL
                mem[cpu->sp-1] = cpu->h;
                mem[cpu->sp-2] = cpu->l;
                cpu->sp += -2;
                break;

            case 0xfe: // CPI D8
                aux = cpu->a;
                SUB(cpu,cpu->a,mem[pc+1])
                cpu->a = aux;
                cpu->pc += 1;
                break;

            default: unimplemented_op(cpu, *op);

        }
    }
}
