#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define REG(n) (((n)==0x7) ? 'A' : (((n)==0x4) ? 'H' : (((n)==0x5) ? 'L' : (n)+'B')))
#define REGPAIR(n) (((n)==0x0) ? "BC" : (((n)==0x1) ? "DE" : (((n)==0x2) ? "HL" : "SP")))
#define CCC(n) (((n)==0x0) ? "NZ" : ((n)==0x1) ? "Z" : ((n)==0x2) ? "NC" : ((n)==0x3) ? "C" : ((n)==0x4) ? "PO" : ((n)==0x5) ? "PE" : ((n)==0x6) ? "P" : "M")

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
        // Data transfer group
        case 0x22: printf("SHLD\t0x%02x%02x", instr[2], instr[1]); len = 3;     break;
        case 0x2a: printf("LHLD\t0x%02x%02x", instr[2], instr[1]); len = 3;     break;
        case 0x32: printf("STA\t0x%02x%02x", instr[2], instr[1]); len = 3;      break;
        case 0x36: printf("MVI\tM\t0x%02x", instr[1]); len = 2;                 break;
        case 0x3a: printf("LDA\t0x%02x%02x", instr[2], instr[1]); len = 3;      break;
        case 0xeb: printf("XCHG");                                              break;
       
        // Arithmetic group
        case 0x27: printf("DAA");                                               break;
        case 0x34: printf("INR\tM");                                            break;
        case 0x35: printf("DCR\tM");                                            break;
        case 0x86: printf("ADD\tM");                                            break;
        case 0x8e: printf("ADC\tM");                                            break;
        case 0x96: printf("SUB\tM");                                            break;
        case 0x9d: printf("SBB\tM");                                            break;
        case 0xc6: printf("ADI\t0x%02x", instr[1]); len = 2;                    break;
        case 0xce: printf("ADC\t0x%02x", instr[1]); len = 2;                    break;
        case 0xd6: printf("SUI\t0x%02x", instr[1]); len = 2;                    break;
        case 0xde: printf("SBI\t0x%02x", instr[1]); len = 2;                    break;
 
        // Logical group
        case 0x07: printf("RLC");                                               break;
        case 0x0f: printf("RRC");                                               break;
        case 0x17: printf("RAL");                                               break;
        case 0x1f: printf("RAR");                                               break;
        case 0x2f: printf("CMA");                                               break;
        case 0x37: printf("STC");                                               break;
        case 0x3f: printf("CMC");                                               break;
        case 0xa6: printf("ANA\tM");                                            break;
        case 0xae: printf("XRA\tM");                                            break;
        case 0xb6: printf("ORA\tM");                                            break;
        case 0xbe: printf("CMP\tM");                                            break;
        case 0xe6: printf("ANI\t0x%02x", instr[1]); len = 2;                    break;
        case 0xee: printf("XRI\t0x%02x", instr[1]); len = 2;                    break;
        case 0xf6: printf("ORI\t0x%02x", instr[1]); len = 2;                    break;
        case 0xfe: printf("CPI\t0x%02x", instr[1]); len = 2;                    break;

        // Branch group
        case 0xc3: printf("JMP\t0x%02x%02x", instr[2], instr[1]); len = 3;      break;
        case 0xc9: printf("RET");                                               break;
        case 0xcd: printf("CALL\t0x%02x%02x", instr[2], instr[1]); len = 3;     break;
        case 0xe9: printf("PCHL");                                              break;

        // Stack, IO and machine control group
        case 0x00: printf("NOP");                                               break;
        case 0x76: printf("HLT");                                               break;
        case 0xd3: printf("OUT\t0x%02x", instr[1]); len = 2;                    break;
        case 0xdb: printf("IN\t0x%02x", instr[1]); len = 2;                     break;
        case 0xe3: printf("XTHL");                                              break;
        case 0xf1: printf("POP\tPSW");                                          break;
        case 0xf3: printf("DI");                                                break;
        case 0xf5: printf("PUSH\tPSW");                                         break;
        case 0xfb: printf("EI");                                                break;
        case 0xf9: printf("SPHL");                                              break;
        

        default:

        // Data transfer group, cont'd
        if      ((c & 0xf8) == 0x70) printf("MOV\tM\t%c", REG(c & 0x7));
        else if ((c & 0xc7) == 0x46) printf("MOV\t%c\tM", REG((c >> 3) & 0x7));
        else if ((c & 0xc0) == 0x40) printf("MOV\t%c\t%c", REG((c >> 3) & 0x7), REG(c & 0x7));
        else if ((c & 0xc7) == 0x06) 
            { printf("MVI\t%c\t0x%02x", REG((c >> 3) & 0x7), instr[1]); len = 2; }
        else if ((c & 0xcf) == 0x01) 
            { printf("LXI\t%s\t0x%02x%02x", REGPAIR((c >> 4) & 0x3), instr[2], instr[1]); len = 3; }
        else if ((c & 0xcf) == 0x0a) printf("LDAX\t%s", REGPAIR((c >> 4) & 0x3));
        else if ((c & 0xcf) == 0x02) printf("STAX\t%s", REGPAIR((c >> 4) & 0x3));

        // Arithmetic group, cont'd
        else if ((c & 0xf8) == 0x80) printf("ADD\t%c", REG(c & 0x7));
        else if ((c & 0xf8) == 0x88) printf("ADC\t%c", REG(c & 0x7));
        else if ((c & 0xf8) == 0x90) printf("SUB\t%c", REG(c & 0x7));
        else if ((c & 0xf8) == 0x98) printf("SBB\t%c", REG(c & 0x7));
        else if ((c & 0xc7) == 0x04) printf("INR\t%c", REG((c >> 3) & 0x7));
        else if ((c & 0xc7) == 0x05) printf("DCR\t%c", REG((c >> 3) & 0x7));
        else if ((c & 0xcf) == 0x03) printf("INX\t%s", REGPAIR((c >> 4) & 0x3));
        else if ((c & 0xcf) == 0x0b) printf("DCX\t%s", REGPAIR((c >> 4) & 0x3));
        else if ((c & 0xcf) == 0x09) printf("DAD\t%s", REGPAIR((c >> 4) & 0x3));

        // Logical group, cont'd
        else if ((c & 0xf8) == 0xa0) printf("ANA\t%c", REG(c & 0x7));
        else if ((c & 0xf8) == 0xa8) printf("XRA\t%c", REG(c & 0x7));
        else if ((c & 0xf8) == 0xb0) printf("ORA\t%c", REG(c & 0x7));
        else if ((c & 0xf8) == 0xb8) printf("CMP\t%c", REG(c & 0x7));

        // Branch group, cont'd
        else if ((c & 0xc7) == 0xc2) 
            { printf("J%s\t0x%02x%02x", CCC((c >> 3) & 0x7), instr[2], instr[1]); len = 3; }
        else if ((c & 0xc7) == 0xc4)
            { printf("C%s\t0x%02x%02x", CCC((c >> 3) & 0x7), instr[2], instr[1]); len = 3; }
        else if ((c & 0xc0) == 0xc0)
            { printf("R%s\t0x%02x%02x", CCC((c >> 3) & 0x7), instr[2], instr[1]); len = 3; }
        else if ((c & 0xc7) == 0xc7) printf("RST\t%d", (c >> 3) & 0x7);

        // Stack, IO and machine control group, cont'd
        else if ((c & 0xcf) == 0xc5) printf("PUSH\t%s", REGPAIR((c >> 4) & 0x3));
        else if ((c & 0xcf) == 0xc1) printf("POP\t%s", REGPAIR((c >> 4) & 0x3));

        else printf("NOP");
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
