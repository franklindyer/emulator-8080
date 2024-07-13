#include <stdio.h>

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

void unimplemented_op(cpu8080* cpu) {
    perror("Unimplemented operation\n");
    exit(1);
}
