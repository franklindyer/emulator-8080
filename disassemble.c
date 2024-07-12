#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define REG(n) (((n)==0x7) ? 'A' : (((n)==0x4) ? 'H' : (((n)==0x5) ? 'L' : (n)+'B')))

/*
    Expects codebuf to point to 8080 instructions
    Reads instruction starting at location pc
    Returns the length of the instruction, in bytes
*/
int disassemble8080(unsigned char *codebuf, int pc) {
    unsigned char *instr = &codebuf[pc];
    unsigned char c = instr[0];
    int instr_bytes = 1;
    printf("%04x ", pc);

    if      ((c & 0xf8) == 0x70) printf("MOV\tM\t%c", REG(c & 0x7));
    else if ((c & 0xc7) == 0x46) printf("MOV\t%c\tM", REG((c >> 3) & 0x7));
    else if ((c & 0xc0) == 0x80) printf("MOV\t%c\t%c", REG((c >> 3) & 0x7), REG(c & 0x7));
    else printf("UNKNOWN");

    printf("\n");
    return instr_bytes;
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
