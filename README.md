Links to things I'm using:

- [Emulator 101](http://emulator101.com)
- [Intel 8080 user's manual](http://bitsavers.trailing-edge.com/components/intel/MCS80/98-153B_Intel_8080_Microcomputer_Systems_Users_Manual_197509.pdf)

How to set up SDL2 library on MacOS:

- Get the [dmg release file](https://www.libsdl.org) from Github, linked on the SDL website
- Copy `SDL2.Framework` into `/Library/Frameworks`
- Use `gcc` to compile with the flags `-F/Library/Frameworks -framework SDL2 -rpath /Library/Frameworks`
- MacOS may block SDL2 since it is from an unknown developer, so you may have to allow it manually in `Privacy & Security` settings
