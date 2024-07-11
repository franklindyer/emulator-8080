CC = gcc
SDL2FLAGS = -F/Library/Frameworks -framework SDL2 -rpath /Library/Frameworks

main: main.c
	$(CC) $(SDL2FLAGS) main.c -o out/main

disassemble: disassemble.c
	$(CC) disassemble.c -o out/disassemble
