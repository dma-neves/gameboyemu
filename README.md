# Gameboy Emulator

![alt text](other/images/super_mario_land.gif)

### Description
  - Work in progress.
  - A Gameboy emulator written in c.
  - Currently passes all individual [blaggr](https://github.com/retrio/gb-test-roms) `cpu_instrs` tests and some games are playable.
  - Regarding cartridge memory controllers only MBC1 has been implemented.

### TODO
  - Add all mbc1 features.
  - mbc > 1.
  - Sound.

### References
  - [Pandocs](https://gbdev.io/pandocs/)
  - [CPU opcode table](https://izik1.github.io/gbops/)
  - [CPU opcode descriptions](https://rgbds.gbdev.io/docs/v0.5.1/gbz80.7)
  - [The Gameboy Emulator Development Guide](https://hacktixme.ga/GBEDG/)
  - Emu dev Discord: Thank you for all the help debugging my countless bugs

### Requirments
  - Unix based OS.
  - gcc compiler.
  - [SDL2](https://www.libsdl.org/)

### Controls
| Keyboard | Gameboy |
| -------- | --------|
| W        | Up      |
| A        | Left    |
| S        | Down    |
| D        | Right   |
| O        | B       |
| P        | A       |
| K        | select  |
| L        | start   |

### Images

|   |   |
|:---:|:---:|
| ![alt text](other/images/boot.png) | ![alt text](other/images/blargg.png) |
| Nintendo's boot sequence | Blargg's cpu_instrs tests |
| ![alt text](other/images/tetris_menu.png) | ![alt text](other/images/tetris.png) |
| Tetris menu | Tetris gameplay |
| ![alt text](other/images/drmario.png) | ![alt text](other/images/super_mario_land.png)
| Dr. Mario | Super Mario Land |
