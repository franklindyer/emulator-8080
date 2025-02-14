CC = gcc
SDL2FLAGS = -F/Library/Frameworks -framework SDL2 -rpath /Library/Frameworks
CFLAGS = -I./src

.PHONY: disassemble space_invaders lunar_rescue sokoban

space_invaders: space_invaders/main.c
	$(CC) $(SDL2FLAGS) $(CFLAGS) space_invaders/main.c -o out/space_invaders

lunar_rescue: lunar_rescue/main.c
	$(CC) $(SDL2FLAGS) $(CFLAGS) lunar_rescue/main.c -o out/lunar_rescue

sokoban: sokoban/main.c
	$(CC) $(SDL2FLAGS) $(CFLAGS) sokoban/main.c -o out/sokoban

disassemble: src/disassemble.c
	$(CC) $(CFLAGS) src/disassemble.c -o out/disassemble
