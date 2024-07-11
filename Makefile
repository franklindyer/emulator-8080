CC = gcc

disassemble: disassemble.c
	$(CC) disassemble.c -o out/disassemble
