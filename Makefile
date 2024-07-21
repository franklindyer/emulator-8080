CC = gcc
SDL2FLAGS = -F/Library/Frameworks -framework SDL2 -rpath /Library/Frameworks
CFLAGS = -I./src

.PHONY: disassemble space_invaders

space_invaders: space_invaders/main.c
	$(CC) $(SDL2FLAGS) $(CFLAGS) space_invaders/main.c -o out/space_invaders

disassemble: src/disassemble.c
	$(CC) $(CFLAGS) src/disassemble.c -o out/disassemble
