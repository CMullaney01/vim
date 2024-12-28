#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <fstream>
#include <SDL2/SDL.h>

class Chip8
{
public:
    // Memory and registers
    uint8_t memory[4096]; // 4KB of memory
    uint8_t V[16];        // General-purpose registers (V0 to VF)
    uint16_t I;           // memory address pointer
    uint16_t pc;          // Program counter
    uint8_t stack[16];    // Stack (16 levels)
    uint8_t sp;           // Stack pointer

    // Timers
    uint8_t delayTimer; // Delay timer
    uint8_t soundTimer; // Sound timer

    // Keypad state
    uint8_t keymap[16];

    // Rendering
    uint8_t gfx[64 * 32]; // screen
    bool drawFlag;
    int WIDTH = 64;
    int HEIGHT = 32;
    int SCALE = 10;

    // Constructor
    Chip8(int SCREEN_WIDTH, int SCREEN_HEIGHT, int SCALE);

    bool loadROM(const char *filepath);
    void initialiseFontset();
    void emulateCycle();
    void updateTimers();
    void handleInput(SDL_Event &e);
    void reset();

private:
    uint16_t fetchOpcode();
    void decodeAndExecute(uint16_t opcode);

    // Access more detailed opcode info instructions from the README.md
    // Opcode          Standard Chip-8 Instructions
    void op_0nnn(uint16_t opcode); // SYS addr
    void op_00E0(uint16_t opcode); // CLS
    void op_00EE(uint16_t opcode); // RET
    void op_1nnn(uint16_t opcode); // JP addr
    void op_2nnn(uint16_t opcode); // CALL addr
    void op_3XKK(uint16_t opcode); // SE Vx, byte
    void op_4XKK(uint16_t opcode); // SNE Vx, byte
    void op_5XY0(uint16_t opcode); // SE Vx, Vy
    void op_6XKK(uint16_t opcode); // LD Vx, byte
    void op_7XKK(uint16_t opcode); // ADD Vx, byte
    void op_8XY0(uint16_t opcode); // LD Vx, Vy
    void op_8XY1(uint16_t opcode); // OR Vx, Vy
    void op_8XY2(uint16_t opcode); // AND Vx, Vy
    void op_8XY3(uint16_t opcode); // XOR Vx, Vy
    void op_8XY4(uint16_t opcode); // ADD Vx, Vy
    void op_8XY5(uint16_t opcode); // SUB Vx, Vy
    void op_8XY6(uint16_t opcode); // SHR Vx {, Vy}
    void op_8XY7(uint16_t opcode); // SUBN Vx, Vy
    void op_8XYE(uint16_t opcode); // SHL Vx {, Vy}
    void op_9XY0(uint16_t opcode); // SNE Vx, Vy
    void op_ANNN(uint16_t opcode); // LD I, addr
    void op_BNNN(uint16_t opcode); // JP V0, addr
    void op_CXKK(uint16_t opcode); // RND Vx, byte
    void op_DXYN(uint16_t opcode); // DRW Vx, Vy, nibble
    void op_EX9E(uint16_t opcode); // SKP Vx
    void op_EXA1(uint16_t opcode); // SKNP Vx
    void op_FX07(uint16_t opcode); // LD Vx, DT
    void op_FX0A(uint16_t opcode); // LD Vx, K
    void op_FX15(uint16_t opcode); // LD DT, Vx
    void op_FX18(uint16_t opcode); // LD ST, Vx
    void op_FX1E(uint16_t opcode); // ADD I, Vx
    void op_FX29(uint16_t opcode); // LD F, Vx
    void op_FX33(uint16_t opcode); // LD B, Vx
    void op_FX55(uint16_t opcode); // LD [I], Vx
    void op_FX65(uint16_t opcode); // LD Vx, [I]
};

#endif // CHIP8_H
