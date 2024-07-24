You need the SDL2 graphics library for C before you can build this program. Here's how to set up SDL2 library on MacOS:

- Get the [dmg release file](https://www.libsdl.org) from Github, linked on the SDL website
- Copy `SDL2.Framework` into `/Library/Frameworks`
- Use `gcc` to compile with the flags `-F/Library/Frameworks -framework SDL2 -rpath /Library/Frameworks`
- MacOS may block SDL2 since it is from an unknown developer, so you may have to allow it manually in `Privacy & Security` settings

Here's how to use this code:

- To build Space Invaders, run `make space_invaders`
- To play Space Invaders, run `./out/space_invaders` from the repo root
    - `A/D` to move left/right
    - `W` to shoot
    - `C` to deposit coin
    - `1/2` to select number of players
- To build Lunar Rescue, run `make lunar_rescue`
- To play Lunar Rescue, run `./out/lunar_rescue` from the repo root
    - `A/D` to move left/right
    - `W` to thrust or shoot
    - `C` to deposit coin
    - `1/2` to select number of players

Credit where credit is due:

- [Emulator 101](http://emulator101.com)
- [Intel 8080 user's manual](http://bitsavers.trailing-edge.com/components/intel/MCS80/98-153B_Intel_8080_Microcomputer_Systems_Users_Manual_197509.pdf)
- Don't remember where I downloaded the Space Invaders sounds
- [Kold666's samples for Lunar Rescue](https://samples.mameworld.info) from MAMEWorld
