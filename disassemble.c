#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
    Expects codebuf to point to 8080 instructions
    Reads instruction starting at location pc
    Returns the length of the instruction, in bytes
*/
int disassemble8080(unsigned char *codebuf, int pc) {
    unsigned char *instr = &codebuf[pc];
    int instr_bytes = 1;
    printf("%04x ", pc);

    printf("UNKNOWN");

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
