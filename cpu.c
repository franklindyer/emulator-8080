#include <stdio.h>

// These macros for use only inside the main case switch of emulate_cpu8080
#define INR(cpu,r) \
    r += 1; \
    SETZSP(cpu->flags,r)
#define DCR(cpu,r) \
    r += -1; \
    (cpu->flags).z = (r == 0); \
    (cpu->flags).s = r >> 7; \
    SETPARITY((cpu->flags).p, r) \
    (cpu->flags).ac = (r & 0xf) == 0xf;
#define ADD(cpu,r) \
    cpu->a = cpu->a + r; \
    SETZSP(cpu->flags,cpu->a) \
    (cpu->flags).c = cpu->a < r;
#define SUB(cpu,a,r) \
    (cpu->flags).z = (a == r); \
    (cpu->flags).c = (a < r); \
    a = a - r; \
    (cpu->flags).s = (a & 0x80) >> 7; \
    SETPARITY((cpu->flags).p, a)
#define ANA(cpu,r) \
    cpu->a = cpu->a & r; \
    SETZSP(cpu->flags,cpu->a) \
    (cpu->flags).c = 0; \
    (cpu->flags).ac = 0;
#define XRA(cpu,r) \
    cpu->a = cpu->a ^ r; \
    SETZSP(cpu->flags,cpu->a) \
    (cpu->flags).c = 0; \
    (cpu->flags).ac = 0;
#define ORA(cpu,r) \
    cpu->a = cpu->a | r; \
    SETZSP(cpu->flags,cpu->a) \
    (cpu->flags).c = 0; \
    (cpu->flags).ac = 0;
#define RST(cpu,n) \
    cpu->memory[cpu->sp-1] = cpu->pc >> 8; \
    cpu->memory[cpu->sp-2] = cpu->pc & 0xff; \
    cpu->sp += -2; \
    cpu->pc = n << 3;
#define MOVCASE(code,r,s) \
    case code: \
        r = s; \
        break;
#define MOVMCASE(code,r,cpu) \
    case code: \
        aux = (cpu->h << 8) | cpu->l; \
        r = cpu->memory[aux]; \
        break;
#define MOVTOMCASE(code,r,cpu) \
    case code: \
        aux = (cpu->h << 8) | cpu->l; \
        cpu->memory[aux] = r; \
        break;
#define CALL(cpu) \
    mem[cpu->sp-1] = (pc+3) >> 8; \
    mem[cpu->sp-2] = (pc+3) & 0xff; \
    cpu->sp += -2; \
    cpu->pc = (mem[pc+2] << 8) + mem[pc+1];

#define PUSHBYTE(cpu,b) \
    mem[cpu->sp-1] = b; \
    cpu->sp += -1;
#define POPBYTE(cpu,r) \
    r = mem[cpu->sp]; \
    cpu->sp += 1;

#define PUSHPSW(cpu) \
    mem[cpu->sp-1] = cpu->a; \
    aux = 0x01; \
    aux = aux | (cpu->flags).c | ((cpu->flags).p << 2) | ((cpu->flags).ac << 4); \
    aux = aux | ((cpu->flags).z << 6) | ((cpu->flags).s << 7); \
    mem[cpu->sp-2] = aux & 0xff; \
    cpu->sp += -2;
#define POPPSW(cpu) \
    cpu->a = mem[cpu->sp+1]; \
    (cpu->flags).c = mem[cpu->sp] & 1; \
    (cpu->flags).p = (mem[cpu->sp] >> 2) & 1; \
    (cpu->flags).ac = (mem[cpu->sp] >> 4) & 1; \
    (cpu->flags).z = (mem[cpu->sp] >> 6) & 1; \
    (cpu->flags).s = (mem[cpu->sp] >> 7) & 1; \
    cpu->sp += 2;

#define SETZSP(flg,x) (flg).z = (x) == 0; (flg).s = (x >> 7) & 1; SETPARITY((flg).p,x)
#define SETPARITY(p,x) aux8=x; aux8=(aux8>>4)^(aux8&0xf); aux8=(aux8>>2)^(aux8&3); aux8=(aux8>>1)^(aux8&1); p=aux8; 

typedef struct flags8080 {
    unsigned char z;
    unsigned char s;
    unsigned char p;
    unsigned char c;
    unsigned char ac;
    unsigned char ei;
    unsigned char i;
} flags8080;

typedef struct cpu8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint8_t bus;
    uint16_t lastpc;
    uint16_t pc;
    uint16_t sp;
    flags8080 flags;
    uint8_t* memory;
    long ticks;
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
    uint8_t aux8;
    uint16_t aux, aux2;
    while (bound > 0) {
        bound--;
        cpu->ticks++;
        pc = cpu->pc;
        if (cpu->flags.i) {
            op = &cpu->bus;
            cpu->flags.i = 0;
        }
        else {
            op = &cpu->memory[cpu->pc];
            cpu->pc++;
        }
        switch(*op) {

            case 0x00: // NOP
                break;

            case 0x01: // LXI BC D16
                cpu->b = mem[pc+2];
                cpu->c = mem[pc+1];
                cpu->pc += 2;
                break;

            case 0x03: // INX BC
                aux = (cpu->b << 8) | cpu->c;
                aux++;
                cpu->b = aux >> 8;
                cpu->c = aux & 255;
                break;

            case 0x04: // INR B
                INR(cpu,cpu->b)
                break;

            case 0x05: // DCR B
                DCR(cpu,cpu->b)
                break;

            case 0x06: // MVI B D8
                cpu->b = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x07: // RLC
                (cpu->flags).c = cpu->a >> 7;
                cpu->a = (cpu->a >> 7) | (cpu->a << 1);
                break;

            case 0x09: // DAD BC
                aux = (cpu->h << 8) | cpu->l;
                aux2 = aux;
                aux += (cpu->b << 8) | cpu->c;
                cpu->h = aux >> 8;
                cpu->l = aux & 0xff;
                (cpu->flags).c = aux2 > aux;
                break;

            case 0x0a: // LDAX BC
                aux = (cpu->b << 8) | cpu->c;
                cpu->a = mem[aux];
                break;

            case 0x0b: // DCX BC
                aux = (cpu->b << 8) | cpu->c;
                aux--;
                cpu->b = aux >> 8;
                cpu->c = aux & 255;
                break;
            
            case 0x0c: // INR C
                INR(cpu,cpu->c)
                break;

            case 0x0d: // DCR C
                DCR(cpu,cpu->c)
                break;

            case 0x0e: // MVI C D8
                cpu->c = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x0f: // RRC
                (cpu->flags).c = 1 == (cpu->a & 1);
                cpu->a = ((cpu->a & 1) << 7) | (cpu->a >> 1);
                break;

            case 0x11: // LXI DE D16
                cpu->d = mem[pc+2];
                cpu->e = mem[pc+1];
                cpu->pc += 2;
                break;

            case 0x13: // INX DE
                aux = (cpu->d << 8) | cpu->e;
                aux++;
                cpu->d = aux >> 8;
                cpu->e = aux & 255;
                break;

            case 0x14: // INR D
                INR(cpu,cpu->d)
                break;

           case 0x15: // DCR D
                DCR(cpu,cpu->d)
                break;

            case 0x16: // MVI D D8
                cpu->d = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x17: // RAL
                aux = cpu->a;
                cpu->a = (cpu->a << 1) | (cpu->flags).c;
                (cpu->flags).c = (aux >> 7) & 1;
                break;

            case 0x1a: // LDAX DE
                aux = (cpu->d << 8) | cpu->e;
                cpu->a = mem[aux];
                break;

            case 0x1b: // DCX DE
                aux = (cpu->d << 8) | cpu->e;
                aux--;
                cpu->d = aux >> 8;
                cpu->e = aux & 255;
                break;
            
            case 0x1c: // INR E
                INR(cpu,cpu->e)
                break;

            case 0x1d: // DCR E
                DCR(cpu,cpu->e)
                break;

            case 0x1e: // MVI E D8
                cpu->e = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x19: // DAD DE
                aux = (cpu->h << 8) | cpu->l;
                aux2 = aux;
                aux += (cpu->d << 8) | cpu->e;
                cpu->h = aux >> 8;
                cpu->l = aux & 0xff;
                (cpu->flags).c = aux2 > aux;
                break;

            case 0x1f: // RAR
                aux = cpu->a;
                cpu->a = (cpu->a >> 1) | ((cpu->flags).c << 7);
                (cpu->flags).c = aux & 1;
                break;

            case 0x21: // LXI HL D16
                cpu->h = mem[pc+2];
                cpu->l = mem[pc+1];
                cpu->pc += 2;
                break;

            case 0x22: // SHLD D16
                aux = (mem[pc+2] << 8) | mem[pc+1];
                mem[aux] = cpu->l;
                mem[aux+1] = cpu->h;
                cpu->pc += 2;
                break;

            case 0x23: // INX HL
                aux = (cpu->h << 8) | cpu->l;
                aux++;
                cpu->h = aux >> 8;
                cpu->l = aux & 255;
                break;

            case 0x24: // INR H
                INR(cpu,cpu->h)
                break;

            case 0x25: // DCR H
                DCR(cpu,cpu->h)
                break;

            case 0x26: // MVI H D8
                cpu->h = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x29: // DAD HL
                (cpu->flags).c = cpu->h >> 7;
                cpu->h = (cpu->h << 1) | (cpu->l >> 7);
                cpu->l = cpu->l << 1;
                break;

            case 0x2a: // LHLD D16
                aux = (mem[pc+2] << 8) | mem[pc+1];
                cpu->h = mem[aux+1];
                cpu->l = mem[aux];
                cpu->pc += 2;
                break;

            case 0x2b: // DCX HL
                aux = (cpu->h << 8) | cpu->l;
                aux--;
                cpu->h = aux >> 8;
                cpu->l = aux & 255;
                break;
            
            case 0x2c: // INR L
                INR(cpu,cpu->l)
                break;

            case 0x2d: // DCR L
                DCR(cpu,cpu->l)
                break;

            case 0x2e: // MVI L D8
                cpu->l = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x31: // LXI SP D16
                cpu->sp = (mem[pc+2] << 8) | mem[pc+1];
                cpu->pc += 2;
                break;

            case 0x32: // STA D16
                aux = (mem[pc+2] << 8) | mem[pc+1];
                mem[aux] = cpu->a;
                cpu->pc += 2;
                break;

            case 0x33: // INX SP
                cpu->sp++;
                break;

            case 0x34: // INR M
                aux = (cpu->h << 8) | cpu->l;
                INR(cpu,mem[aux])
                break;

            case 0x35: // DCR M
                aux = (cpu->h << 8) | cpu->l;
                DCR(cpu,mem[aux])
                break;

            case 0x36: // MVI M D8
                aux = (cpu->h << 8) | cpu->l;
                mem[aux] = mem[pc+1];
                cpu->pc += 1;
                break;

            case 0x37: // STC
                (cpu->flags).c = 1;
                break;

            case 0x39: // DAD SP
                aux = (cpu->h << 8) | cpu->l;
                aux += cpu->sp;
                cpu->h = aux >> 8;
                cpu->l = aux & 0xff;
                (cpu->flags).c = cpu->sp > aux;
                break;

            case 0x3a: // LDA D16
                aux = (mem[pc+2] << 8) | mem[pc+1];
                cpu->a = mem[aux];
                cpu->pc += 2;
                break;

            case 0x3b: // DCX SP
                cpu->sp--;
                break;
            
            case 0x3c: // INR A
                INR(cpu,cpu->a)
                break;

            case 0x3d: // DCR A
                DCR(cpu,cpu->a)
                break;

            case 0x3e: // MVI A D8
                cpu->a = mem[pc+1];
                cpu->pc += 1;
                break;

            MOVCASE(0x40,cpu->b,cpu->b) // MOV B B
            MOVCASE(0x41,cpu->b,cpu->c) // MOV B C
            MOVCASE(0x42,cpu->b,cpu->d) // MOV B D
            MOVCASE(0x43,cpu->b,cpu->e) // MOV B E
            MOVCASE(0x44,cpu->b,cpu->h) // MOV B H
            MOVCASE(0x45,cpu->b,cpu->l) // MOV B L
            MOVMCASE(0x46,cpu->b,cpu) // MOV B M
            MOVCASE(0x47,cpu->b,cpu->a) // MOV B A

            MOVCASE(0x48,cpu->c,cpu->b) // MOV C B
            MOVCASE(0x49,cpu->c,cpu->c) // MOV C C
            MOVCASE(0x4a,cpu->c,cpu->d) // MOV C D
            MOVCASE(0x4b,cpu->c,cpu->e) // MOV C E
            MOVCASE(0x4c,cpu->c,cpu->h) // MOV C H
            MOVCASE(0x4d,cpu->c,cpu->l) // MOV C L
            MOVMCASE(0x4e,cpu->c,cpu) // MOV C M
            MOVCASE(0x4f,cpu->c,cpu->a) // MOV C A

            MOVCASE(0x50,cpu->d,cpu->b) // MOV D B
            MOVCASE(0x51,cpu->d,cpu->c) // MOV D C
            MOVCASE(0x52,cpu->d,cpu->d) // MOV D D
            MOVCASE(0x53,cpu->d,cpu->e) // MOV D E
            MOVCASE(0x54,cpu->d,cpu->h) // MOV D H
            MOVCASE(0x55,cpu->d,cpu->l) // MOV D L
            MOVMCASE(0x56,cpu->d,cpu) // MOV D M
            MOVCASE(0x57,cpu->d,cpu->a) // MOV D A

            MOVCASE(0x58,cpu->e,cpu->b) // MOV E B
            MOVCASE(0x59,cpu->e,cpu->c) // MOV E C
            MOVCASE(0x5a,cpu->e,cpu->d) // MOV E D
            MOVCASE(0x5b,cpu->e,cpu->e) // MOV E E
            MOVCASE(0x5c,cpu->e,cpu->h) // MOV E H
            MOVCASE(0x5d,cpu->e,cpu->l) // MOV E L
            MOVMCASE(0x5e,cpu->e,cpu) // MOV E M
            MOVCASE(0x5f,cpu->e,cpu->a) // MOV E A

            MOVCASE(0x60,cpu->h,cpu->b) // MOV H B
            MOVCASE(0x61,cpu->h,cpu->c) // MOV H C
            MOVCASE(0x62,cpu->h,cpu->d) // MOV H D
            MOVCASE(0x63,cpu->h,cpu->e) // MOV H E
            MOVCASE(0x64,cpu->h,cpu->h) // MOV H H
            MOVCASE(0x65,cpu->h,cpu->l) // MOV H L
            MOVMCASE(0x66,cpu->h,cpu) // MOV H M
            MOVCASE(0x67,cpu->h,cpu->a) // MOV H A

            MOVCASE(0x68,cpu->l,cpu->b) // MOV L B
            MOVCASE(0x69,cpu->l,cpu->c) // MOV L C
            MOVCASE(0x6a,cpu->l,cpu->d) // MOV L D
            MOVCASE(0x6b,cpu->l,cpu->e) // MOV L E
            MOVCASE(0x6c,cpu->l,cpu->h) // MOV L H
            MOVCASE(0x6d,cpu->l,cpu->l) // MOV L L
            MOVMCASE(0x6e,cpu->l,cpu) // MOV L M
            MOVCASE(0x6f,cpu->l,cpu->a) // MOV L A

            MOVTOMCASE(0x70,cpu->b,cpu) // MOV M B
            MOVTOMCASE(0x71,cpu->c,cpu) // MOV M C
            MOVTOMCASE(0x72,cpu->d,cpu) // MOV M D
            MOVTOMCASE(0x73,cpu->e,cpu) // MOV M E
            MOVTOMCASE(0x74,cpu->h,cpu) // MOV M H
            MOVTOMCASE(0x75,cpu->l,cpu) // MOV M L
            MOVTOMCASE(0x77,cpu->a,cpu) // MOV M A

            case 0x76: // HLT
                return;

            MOVCASE(0x78,cpu->a,cpu->b) // MOV A B
            MOVCASE(0x79,cpu->a,cpu->c) // MOV A C
            MOVCASE(0x7a,cpu->a,cpu->d) // MOV A D
            MOVCASE(0x7b,cpu->a,cpu->e) // MOV A E
            MOVCASE(0x7c,cpu->a,cpu->h) // MOV A H
            MOVCASE(0x7d,cpu->a,cpu->l) // MOV A L
            MOVMCASE(0x7e,cpu->a,cpu) // MOV A M
            MOVCASE(0x7f,cpu->a,cpu->a) // MOV A A

            case 0x80: // ADD B
                ADD(cpu,cpu->b)
                break; 

            case 0x81: // ADD C
                ADD(cpu,cpu->c)
                break; 

            case 0x82: // ADD D
                ADD(cpu,cpu->d)
                break; 

            case 0x83: // ADD E
                ADD(cpu,cpu->e)
                break; 

            case 0x84: // ADD H
                ADD(cpu,cpu->h)
                break; 

            case 0x85: // ADD L
                ADD(cpu,cpu->l)
                break; 

            case 0x86: // ADD M
                aux = (cpu->h << 8) | cpu->l;
                ADD(cpu,mem[aux])
                break; 

            case 0x87: // ADD A
                ADD(cpu,cpu->a)
                break; 

            case 0xa0: // ANA B
                ANA(cpu,cpu->b)
                break;

            case 0xa1: // ANA C
                ANA(cpu,cpu->c)
                break;

            case 0xa2: // ANA D
                ANA(cpu,cpu->d)
                break;

            case 0xa3: // ANA E
                ANA(cpu,cpu->e)
                break;

            case 0xa4: // ANA H
                ANA(cpu,cpu->h)
                break;

            case 0xa5: // ANA L
                ANA(cpu,cpu->l)
                break;

            case 0xa6: // ANA M
                aux = (cpu->h << 8) | cpu->l;
                ANA(cpu,mem[aux])
                break;
            
            case 0xa7: // ANA A
                ANA(cpu,cpu->a)
                break;

            case 0xa8: // XRA B
                XRA(cpu,cpu->b)
                break;

            case 0xa9: // XRA C
                XRA(cpu,cpu->c)
                break;

            case 0xaa: // XRA D
                XRA(cpu,cpu->d)
                break;

            case 0xab: // XRA E
                XRA(cpu,cpu->e)
                break;

            case 0xac: // XRA H
                XRA(cpu,cpu->h)
                break;

            case 0xad: // XRA L
                XRA(cpu,cpu->l)
                break;

            case 0xae: // XRA M
                aux = (cpu->h << 8) | cpu->l;
                XRA(cpu,mem[aux])
                break;

            case 0xaf: // XRA A
                cpu->a = 0;
                (cpu->flags).z = 0;
                (cpu->flags).s = 0;
                (cpu->flags).p = 0;
                (cpu->flags).c = 0;
                (cpu->flags).ac = 0;
                break;

            case 0xb0: // ORA B
                ORA(cpu,cpu->b)
                break;

            case 0xb1: // ORA C
                ORA(cpu,cpu->c)
                break;

            case 0xb2: // ORA D
                ORA(cpu,cpu->d)
                break;

            case 0xb3: // ORA E
                ORA(cpu,cpu->e)
                break;

            case 0xb4: // ORA H
                ORA(cpu,cpu->h)
                break;

            case 0xb5: // ORA L
                ORA(cpu,cpu->l)
                break;

            case 0xb6: // ORA M
                aux = (cpu->h << 8) | cpu->l;
                ORA(cpu,mem[aux])
                break;

            case 0xb7: // ORA A
                ORA(cpu,cpu->a)
                break;

            case 0xb8: // CMP B
                aux = cpu->a;
                SUB(cpu,cpu->a,cpu->b)
                cpu->a = aux;
                break;

            case 0xb9: // CMP C
                aux = cpu->a;
                SUB(cpu,cpu->a,cpu->c)
                cpu->a = aux;
                break;

            case 0xba: // CMP D
                aux = cpu->a;
                SUB(cpu,cpu->a,cpu->d)
                cpu->a = aux;
                break;

            case 0xbb: // CMP E
                aux = cpu->a;
                SUB(cpu,cpu->a,cpu->e)
                cpu->a = aux;
                break;

            case 0xbc: // CMP H
                aux = cpu->a;
                SUB(cpu,cpu->a,cpu->h)
                cpu->a = aux;
                break;

            case 0xbd: // CMP L
                aux = cpu->a;
                SUB(cpu,cpu->a,cpu->l)
                cpu->a = aux;
                break;

            case 0xbe: // CMP M
                aux = cpu->a;
                SUB(cpu,cpu->a,mem[(cpu->h << 8) | cpu->l])
                cpu->a = aux;
                break;

            case 0xbf: // CMP A
                aux = cpu->a;
                SUB(cpu,cpu->a,cpu->a)
                cpu->a = aux;
                break;

            case 0xc0: // RNZ D16
                if ((cpu->flags).z) break; 
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xc1: // POP BC
                cpu->b = mem[cpu->sp+1];
                cpu->c = mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xc2: // JNZ D16
                if (!(cpu->flags).z) cpu->pc = (mem[pc+2] << 8) + mem[pc+1];
                else cpu->pc += 2;
                break; 

            case 0xc3: // JMP D16
                cpu->pc = (mem[pc+2] << 8) + mem[pc+1];
                break;

            case 0xc4: // CNZ D16
                if (!(cpu->flags).z) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xc5: // PUSH BC
                mem[cpu->sp-1] = cpu->b;
                mem[cpu->sp-2] = cpu->c;
                cpu->sp += -2;
                break;

            case 0xc6: // ADI D8
                aux = cpu->a;
                aux += mem[pc+1];
                cpu->a = aux;
                SETZSP(cpu->flags,cpu->a)
                (cpu->flags).c = (aux >> 8) & 1;
                // AC flag not implemented
                cpu->pc += 1;
                break;

            case 0xc7: // RST 0
                RST(cpu,0)
                break;

            case 0xc8: // RZ
                if (!(cpu->flags).z) break;

            case 0xc9: // RET
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xca: // JZ D16
                if ((cpu->flags).z) cpu->pc = (mem[pc+2] << 8) + mem[pc+1];
                else cpu->pc += 2;
                break;

            case 0xcc: // CZ D16
                if ((cpu->flags).z) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xcd: // CALL D16
                CALL(cpu)
                break;
           
            case 0xcf: // RST 1
                RST(cpu,1)
                break;
 
            case 0xd0: // RNC
                if ((cpu->flags).c) break; 
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xd1: // POP DE
                cpu->d = mem[cpu->sp+1];
                cpu->e = mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xd2: // JNC D16
                if (!(cpu->flags).c) cpu->pc = (mem[pc+2] << 8) | mem[pc+1];
                else cpu->pc += 2;
                break;

            case 0xd3: // OUT
                // UNIMPLEMENTED FOR NOW
                cpu->pc += 1;
                break;

            case 0xd4: // CNC D16
                if (!(cpu->flags).c) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xd5: // PUSH DE
                mem[cpu->sp-1] = cpu->d;
                mem[cpu->sp-2] = cpu->e;
                cpu->sp += -2;
                break;

            case 0xd6: // SUI D8
                cpu->a = cpu->a - mem[pc+1];
                SETZSP(cpu->flags,cpu->a)
                (cpu->flags).c = cpu->a > mem[pc+1];
                cpu->pc += 1;
                break;

            case 0xd7: // RST 2
                RST(cpu,2)
                break;

            case 0xd8: // RC
                if (!(cpu->flags).c) break; 
                cpu->pc = (mem[cpu->sp+1] << 8) | mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xda: // JC D16
                if ((cpu->flags).c) cpu->pc = (mem[pc+2] << 8) | mem[pc+1];
                else cpu->pc += 2;
                break;

            case 0xdb: // IN
                // UNIMPLEMENTED FOR NOW
                cpu->pc += 1;
                break;

            case 0xdc: // CC D16
                if ((cpu->flags).c) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xde: // SBI D8
                (cpu->flags).c = cpu->a < mem[pc+1] + (cpu->flags).c;
                cpu->a = cpu->a - (mem[pc+1] + (cpu->flags).c);
                SETZSP(cpu->flags,cpu->a)
                // AC flag not implemented yet
                cpu->pc += 1;
                break;

            case 0xdf: // RST 3
                RST(cpu,3)
                break;

            case 0xe0: // RPO
                if (!(cpu->flags).p) break; 
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xe1: // POP HL
                cpu->h = mem[cpu->sp+1];
                cpu->l = mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xe2: // JPO D16
                if ((cpu->flags).p) cpu->pc = (mem[pc+2] << 8) | mem[pc+1];
                else cpu->pc += 2;
                break;

            case 0xe3: // XTHL
                aux = (mem[cpu->sp+1] << 8) | mem[cpu->sp];
                mem[cpu->sp] = cpu->l;
                mem[cpu->sp+1] = cpu->h;
                cpu->h = aux >> 8;
                cpu->l = aux & 0xff;
                break;

            case 0xe4: // CPO D16
                if ((cpu->flags).p) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xe5: // PUSH HL
                mem[cpu->sp-1] = cpu->h;
                mem[cpu->sp-2] = cpu->l;
                cpu->sp += -2;
                break;

            case 0xe6: // ANI D8
                cpu->a = cpu->a & mem[pc+1];
                SETZSP(cpu->flags,cpu->a)
                (cpu->flags).c = 0;
                (cpu->flags).ac = 0;
                cpu->pc += 1;
                break;

            case 0xe7: // RST 4
                RST(cpu,4)
                break;

            case 0xe8: // RPE
                if ((cpu->flags).p) break; 
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xe9: // PCHL
                cpu->pc = (cpu->h << 8) | cpu->l;
                break;

            case 0xea: // JPE
                if (!(cpu->flags).p) cpu->pc = (mem[pc+2] << 8) | mem[pc+1];
                else cpu->pc += 2;
                break;

            case 0xeb: // XCHG
                aux = (cpu->h << 8) | cpu->l;
                cpu->h = cpu->d;
                cpu->l = cpu->e;
                cpu->d = aux >> 8;
                cpu->e = aux & 0xff;
                break;

            case 0xec: // CPE D16
                if (!(cpu->flags).p) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xef: // RST 5
                RST(cpu,5)
                break;

            case 0xf0: // RP
                if ((cpu->flags).s) break; 
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xf1: // POP PSW
                POPPSW(cpu)
                break;

            case 0xf2: // JP D16
                if (!(cpu->flags).s) cpu->pc = (mem[pc+2] << 8) | mem[pc+1];
                else cpu->pc += 2;
                break;

            case 0xf3: // DI
                (cpu->flags).ei = 0;
                break;

            case 0xf4: // CP D16
                if (!(cpu->flags).s) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xf5: // PUSH PSW
                PUSHPSW(cpu)
                break;

            case 0xf6: // ORI D8
                cpu->a = cpu->a | mem[pc+1];
                SETZSP(cpu->flags,cpu->a)
                (cpu->flags).c = 0;
                (cpu->flags).ac = 0;
                cpu->pc += 1;
                break;

            case 0xf7: // RST 6
                RST(cpu,6)
                break;

            case 0xf8: // RM
                if (!(cpu->flags).s) break; 
                cpu->pc = (mem[cpu->sp+1] << 8) + mem[cpu->sp];
                cpu->sp += 2;
                break;

            case 0xfa: // JM D16
                if ((cpu->flags).s) cpu->pc = (mem[pc+2] << 8) | mem[pc+1];
                else cpu->pc += 2;
                break; 

            case 0xfb: // EI
                (cpu->flags).ei = 1;
                break;

            case 0xfc: // CM D16
                if ((cpu->flags).s) { CALL(cpu) }
                else cpu->pc += 2;
                break;

            case 0xfe: // CPI D8
                aux = cpu->a;
                SUB(cpu,cpu->a,mem[pc+1])
                cpu->a = aux;
                cpu->pc += 1;
                break;

            case 0xff: // RST 7
                RST(cpu,7)
                break;

            default: unimplemented_op(cpu, *op);

        }
        cpu->lastpc = pc;
    }
}
