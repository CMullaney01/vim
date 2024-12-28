# Learning Vim

I am learning to use Vim with baby steps and as such I have added vim bindings to my IDE I have put together a cheat sheet of some of the motions I expect to use and will build a chip8 emulator in C++ to brush up on my C++ skills in the process.

# Project layout

```
chip8-emulator/
├── README.md
├── src/
│ ├── main.cpp # Entry point for the emulator
│ ├── chip8.cpp # Implementation of the CHIP-8 core
├── include/
│ ├── chip8.h # Definition of the CHIP-8 class
├── build/ # Generated build files (ignored in .gitignore)
├── tests/
│ ├── test_chip8.cpp # Unit tests for the CHIP-8 core
├── roms/ # Folder for CHIP-8 ROMs to load and test
```

# Cowgod's Chip8 technical reference

[Cowgods technical reference](https://github.com/trapexit/chip-8_documentation/blob/master/Misc/Cowgod's%20CHIP-8%20Technical%20Reference.pdf)

## Memory Key Notes

1. 4096 bytes of RAM loc. 0x000 to 0xFFF
2. 0x000 to 0x1FF restricted (off limits) start from 0x200

## Register Key Notes

1. 16 registers
   - VF: reserved for some flag instructions
   - 2 special registers for delay and sound timers
   - PC: program counter (you know how to use this from ACA) 16-bit
   - SP: Stack pointer 8bit ()for stack ref which is 16 16-bit vals

## Instruction Set

36 original instructions implement and add here as needed
