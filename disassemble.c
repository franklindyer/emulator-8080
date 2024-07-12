#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define REG(n) (((n)==0x7) ? 'A' : (((n)==0x4) ? 'H' : (((n)==0x5) ? 'L' : (n)+'B')))
#define REGPAIR(n) (((n)==0x0) ? "BC" : (((n)==0x1) ? "DE" : (((n)==0x2) ? "HL" : "SP")))

/*
    Expects codebuf to point to 8080 instructions
    Reads instruction starting at location pc
    Returns the length of the instruction, in bytes
*/
int disassemble8080(unsigned char *codebuf, int pc) {
    unsigned char *instr = &codebuf[pc];
    unsigned char c = instr[0];
    int len = 1;
    printf("%04x ", pc);

    switch(c) {
        case 0x00: printf("NOP");                                               break;
        case 0x22: printf("SHLD\t0x%02x%02x", instr[2], instr[1]); len = 2;     break;
        case 0x2a: printf("LHLD\t0x%02x%02x", instr[2], instr[1]); len = 2;     break;
        case 0x32: printf("STA\t0x%02x%02x", instr[2], instr[1]); len = 2;      break;
        case 0x36: printf("MVI\tM\t0x%02x", instr[1]); len = 2;                 break;
        case 0x3a: printf("LDA\t0x%02x%02x", instr[2], instr[1]); len = 2;      break;
        case 0xeb: printf("XCHG");                                              break;
        case 0xf3: printf("DI");                                                break;
        case 0xfb: printf("EI");                                                break;
        default:
        if      ((c & 0xf8) == 0x70) printf("MOV\tM\t%c", REG(c & 0x7));
        else if ((c & 0xc7) == 0x46) printf("MOV\t%c\tM", REG((c >> 3) & 0x7));
        else if ((c & 0xc0) == 0x80) printf("MOV\t%c\t%c", REG((c >> 3) & 0x7), REG(c & 0x7));
        else if ((c & 0xc7) == 0x06) 
            { printf("MVI\t%c\t0x%02x", REG((c >> 3) & 0x7), instr[1]); len = 3; }
        else if ((c & 0xcf) == 0x01) 
            { printf("LXI\t%s\t0x%02x%02x", REGPAIR((c >> 4) & 0x3), instr[2], instr[1]); len = 3; }
        else if ((c & 0xcf) == 0x0a) printf("LDAX\t%s", REGPAIR((c >> 4) & 0x3));
        else if ((c & 0xcf) == 0x02) printf("STAX\t%s", REGPAIR((c >> 4) & 0x3));
        else printf("UNKNOWN");
    }

    printf("\n");
    return len;
}

int main(int argc, char **argv) {
    printf("Hello, world!\n");

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("Your file is in another castle. Does it even exist?\n");
        return 1;
    }

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    unsigned char *instrbuf = malloc(size);
    read(fd, instrbuf, size);
    close(fd);

    int pc = 0;
    while (pc < size) pc += disassemble8080(instrbuf, pc);
    return 0;
}
