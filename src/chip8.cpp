#include "../include/chip8.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <SDL2/SDL.h>

const unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8(int SCREEN_WIDTH, int SCREEN_HEIGHT, int WINDOW_SCALE)
{
    SCALE = WINDOW_SCALE;
    HEIGHT = SCREEN_HEIGHT;
    WIDTH = SCREEN_WIDTH;

    memset(memory, 0, sizeof(memory));
    memset(V, 0, sizeof(V));

    pc = 0x200; // Explained in README
    sp = 0;

    delayTimer = 0;
    soundTimer = 0;

    memset(gfx, 0, sizeof(gfx)); // clear screen
}

bool Chip8::loadROM(const char *filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (file)
    {
        file.read(reinterpret_cast<char *>(memory + 0x200), 4096 - 0x200); // Read into memory starting at 0x200
        file.close();
        return true;
    }
    else
    {
        std::cerr << "Failed to load ROM!" << std::endl;
        return false;
    }
}

void Chip8::initialiseFontset()
{
    // Ensure that memory has enough space from 0x050 onwards
    std::copy(std::begin(fontset), std::end(fontset), memory + 0x050);
}

// Update the timers (delay and sound)
void Chip8::updateTimers()
{
    if (delayTimer > 0)
    {
        --delayTimer; // Decrease delay timer
    }
    if (soundTimer > 0)
    {
        --soundTimer; // Decrease sound timer
    }
}

void Chip8::handleInput(SDL_Event &e)
{
    // Define the mapping from CHIP-8 keys to SDL scancodes
    const uint8_t keyMapping[16] = {
        SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
        SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
        SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
        SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};

    // Get the current state of the keyboard
    const uint8_t *state = SDL_GetKeyboardState(NULL);

    // Update the CHIP-8 key states based on the current keyboard state
    for (int i = 0; i < 16; ++i)
    {
        keymap[i] = state[keyMapping[i]] ? 1 : 0; // 1 if pressed, 0 otherwise
    }
}

void Chip8::reset()
{
    // Clear memory
    memset(memory, 0, sizeof(memory));

    // Clear registers
    memset(V, 0, sizeof(V));
    pc = 0x200; // Program counter starts at 0x200

    // Clear stack and stack pointer
    memset(stack, 0, sizeof(stack));
    sp = 0;

    // Reset timers
    delayTimer = 0;
    soundTimer = 0;

    // Clear graphics buffer
    memset(gfx, 0, sizeof(gfx));

    // Clear keypad state
    memset(keymap, 0, sizeof(keymap));

    // Reinitialize the fontset in memory
    initialiseFontset();
}

// Emulate one cycle: fetch, decode, and execute opcode
void Chip8::emulateCycle()
{
    uint16_t opcode = fetchOpcode();
    decodeAndExecute(opcode);
    updateTimers();
}

uint16_t Chip8::fetchOpcode()
{
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1]; // fetch two byes from memory and combine them.
    pc += 2;
    return opcode;
}
void Chip8::decodeAndExecute(uint16_t opcode)
{
    auto handleUnknownOpcode = [&]()
    {
        std::cerr << "Unknown Opcode: " << std::hex << opcode << std::endl;
        // Optionally throw an exception or log error
    };

    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode & 0x000F)
        {
        case 0x0000:
            return op_00E0(opcode);
        case 0x000E:
            return op_00EE(opcode);
        default:
            return handleUnknownOpcode();
        }
    case 0x1000:
        return op_1nnn(opcode);
    case 0x2000:
        return op_2nnn(opcode);
    case 0x3000:
        return op_3XKK(opcode);
    case 0x4000:
        return op_4XKK(opcode);
    case 0x5000:
        return op_5XY0(opcode);
    case 0x6000:
        return op_6XKK(opcode);
    case 0x7000:
        return op_7XKK(opcode);
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000:
            return op_8XY0(opcode);
        case 0x0001:
            return op_8XY1(opcode);
        case 0x0002:
            return op_8XY2(opcode);
        case 0x0003:
            return op_8XY3(opcode);
        case 0x0004:
            return op_8XY4(opcode);
        case 0x0005:
            return op_8XY5(opcode);
        case 0x0006:
            return op_8XY6(opcode);
        case 0x0007:
            return op_8XY7(opcode);
        case 0x000E:
            return op_8XYE(opcode);
        default:
            return handleUnknownOpcode();
        }
    case 0x9000:
        return op_9XY0(opcode);
    case 0xA000:
        return op_ANNN(opcode);
    case 0xB000:
        return op_BNNN(opcode);
    case 0xC000:
        return op_CXKK(opcode);
    case 0xD000:
        return op_DXYN(opcode);
    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E:
            return op_EX9E(opcode);
        case 0x00A1:
            return op_EXA1(opcode);
        default:
            return handleUnknownOpcode();
        }
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            return op_FX07(opcode);
        case 0x000A:
            return op_FX0A(opcode);
        case 0x0015:
            return op_FX15(opcode);
        case 0x0018:
            return op_FX18(opcode);
        case 0x001E:
            return op_FX1E(opcode);
        case 0x0029:
            return op_FX29(opcode);
        case 0x0033:
            return op_FX33(opcode);
        case 0x0055:
            return op_FX55(opcode);
        case 0x0065:
            return op_FX65(opcode);
        default:
            return handleUnknownOpcode();
        }
    default:
        return handleUnknownOpcode();
    }
}

// SYS addr - Ignored on modern interpreters, reserved for old systems
void Chip8::op_0nnn(uint16_t opcode)
{
    // Typically ignored
}

// CLS - Clear the display
void Chip8::op_00E0(uint16_t opcode)
{
    memset(gfx, 0, sizeof(gfx));
}

// RET - Return from a subroutine
void Chip8::op_00EE(uint16_t opcode)
{
    sp--;
    pc = stack[sp];
}

// JP addr - Jump to location nnn
void Chip8::op_1nnn(uint16_t opcode)
{
    pc = opcode & 0x0FFF;
}

// CALL addr - Call subroutine at nnn
void Chip8::op_2nnn(uint16_t opcode)
{
    stack[sp] = pc;
    sp++;
    pc = opcode & 0x0FFF;
}

// SE Vx, byte - Skip next instruction if Vx == kk
void Chip8::op_3XKK(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    uint8_t kk = opcode & 0x00FF;
    pc += (Vx == kk) ? 2 : 0;
}

// SNE Vx, byte - Skip next instruction if Vx != kk
void Chip8::op_4XKK(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    uint8_t kk = opcode & 0x00FF;
    pc += (Vx != kk) ? 2 : 0;
}

// SE Vx, Vy - Skip next instruction if Vx == Vy
void Chip8::op_5XY0(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    uint8_t Vy = V[(opcode & 0x00F0) >> 4];
    pc += (Vx == Vy) ? 2 : 0;
}

// LD Vx, byte - Load byte into Vx
void Chip8::op_6XKK(uint16_t opcode)
{
    V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
}

// ADD Vx, byte - Add byte to Vx
void Chip8::op_7XKK(uint16_t opcode)
{
    V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
}

// LD Vx, Vy - Load Vy into Vx
void Chip8::op_8XY0(uint16_t opcode)
{
    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
}

// OR Vx, Vy - Bitwise OR Vx and Vy
void Chip8::op_8XY1(uint16_t opcode)
{
    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
}

// AND Vx, Vy - Bitwise AND Vx and Vy
void Chip8::op_8XY2(uint16_t opcode)
{
    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
}

// XOR Vx, Vy - Bitwise XOR Vx and Vy
void Chip8::op_8XY3(uint16_t opcode)
{
    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
}

// ADD Vx, Vy - Add Vy to Vx, set VF = carry
void Chip8::op_8XY4(uint16_t opcode)
{
    uint16_t sum = V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4];
    V[0xF] = sum > 0xFF; // Carry flag
    V[(opcode & 0x0F00) >> 8] = sum & 0xFF;
}

// SUB Vx, Vy - Subtract Vy from Vx, set VF = NOT borrow
void Chip8::op_8XY5(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    uint8_t Vy = V[(opcode & 0x00F0) >> 4];
    V[0xF] = Vx > Vy; // NOT borrow
    V[(opcode & 0x0F00) >> 8] -= Vy;
}

// SHR Vx {, Vy} - Shift Vx right by 1, VF = least significant bit
void Chip8::op_8XY6(uint16_t opcode)
{
    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1; // Least significant bit
    V[(opcode & 0x0F00) >> 8] >>= 1;
}

// SUBN Vx, Vy - Set Vx = Vy - Vx, VF = NOT borrow
void Chip8::op_8XY7(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    uint8_t Vy = V[(opcode & 0x00F0) >> 4];
    V[0xF] = Vy > Vx; // NOT borrow
    V[(opcode & 0x0F00) >> 8] = Vy - Vx;
}

// SHL Vx {, Vy} - Shift Vx left by 1, VF = most significant bit
void Chip8::op_8XYE(uint16_t opcode)
{
    V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7; // Most significant bit
    V[(opcode & 0x0F00) >> 8] <<= 1;
}

// SNE Vx, Vy - Skip next instruction if Vx != Vy
void Chip8::op_9XY0(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    uint8_t Vy = V[(opcode & 0x00F0) >> 4];
    pc += (Vx != Vy) ? 2 : 0;
}

// LD I, addr - Set I = nnn
void Chip8::op_ANNN(uint16_t opcode)
{
    I = opcode & 0x0FFF;
}

// JP V0, addr - Jump to location nnn + V0
void Chip8::op_BNNN(uint16_t opcode)
{
    pc = (opcode & 0x0FFF) + V[0];
}

// RND Vx, byte - Set Vx = random byte AND kk
void Chip8::op_CXKK(uint16_t opcode)
{
    uint8_t randomByte = rand() & 0xFF; // Generate random byte
    uint8_t kk = opcode & 0x00FF;
    V[(opcode & 0x0F00) >> 8] = randomByte & kk;
}

// DRW Vx, Vy, nibble - Draw sprite at (Vx, Vy) with width 8 pixels and height n bytes
void Chip8::op_DXYN(uint16_t opcode)
{
    uint8_t x = V[(opcode & 0x0F00) >> 8];
    uint8_t y = V[(opcode & 0x00F0) >> 4];
    uint8_t height = opcode & 0x000F;

    V[0xF] = 0; // Reset collision flag
    for (int row = 0; row < height; ++row)
    {
        uint8_t spriteRow = memory[I + row];
        for (int col = 0; col < 8; ++col)
        {
            uint8_t pixel = (spriteRow >> (7 - col)) & 1;
            uint8_t *screenPixel = &gfx[(x + col + ((y + row) * WIDTH)) % (WIDTH * HEIGHT)];
            if (*screenPixel == 1 && pixel == 1)
                V[0xF] = 1; // Collision detected
            *screenPixel ^= pixel;
        }
    }
    drawFlag = true;
}

// SKP Vx - Skip next instruction if key with the value of Vx is pressed
void Chip8::op_EX9E(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    pc += (keymap[Vx] != 0) ? 2 : 0;
}

// SKNP Vx - Skip next instruction if key with the value of Vx is not pressed
void Chip8::op_EXA1(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    pc += (keymap[Vx] == 0) ? 2 : 0;
}

// LD Vx, DT - Set Vx = delay timer value
void Chip8::op_FX07(uint16_t opcode)
{
    V[(opcode & 0x0F00) >> 8] = delayTimer;
}

// LD DT, Vx - Set delay timer = Vx
void Chip8::op_FX15(uint16_t opcode)
{
    delayTimer = V[(opcode & 0x0F00) >> 8];
}

// LD ST, Vx - Set sound timer = Vx
void Chip8::op_FX18(uint16_t opcode)
{
    soundTimer = V[(opcode & 0x0F00) >> 8];
}

// ADD I, Vx - Set I = I + Vx
void Chip8::op_FX1E(uint16_t opcode)
{
    I += V[(opcode & 0x0F00) >> 8];
}

// LD F, Vx - Set I = location of sprite for digit Vx
void Chip8::op_FX29(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    I = Vx * 5; // Each digit sprite is 5 bytes
}

// LD [I], Vx - Store registers V0 through Vx in memory starting at I
void Chip8::op_FX55(uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (int i = 0; i <= x; ++i)
        memory[I + i] = V[i];
}

// LD Vx, [I] - Read registers V0 through Vx from memory starting at I
void Chip8::op_FX65(uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (int i = 0; i <= x; ++i)
        V[i] = memory[I + i];
}

// LD B, Vx - Store binary-coded decimal representation of Vx in memory locations I, I+1, and I+2
void Chip8::op_FX33(uint16_t opcode)
{
    uint8_t Vx = V[(opcode & 0x0F00) >> 8];
    memory[I] = Vx / 100;           // Hundreds
    memory[I + 1] = (Vx / 10) % 10; // Tens
    memory[I + 2] = Vx % 10;        // Units
}

// Function to wait for a key press and load the key value into Vx
void Chip8::op_FX0A(uint16_t opcode)
{
    // Extract the register index from the opcode
    uint8_t x = (opcode & 0x0F00) >> 8;

    bool keyPressed = false;

    while (!keyPressed)
    {
        // Poll events to check for key presses
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                // Handle quitting if necessary
                return;
            }

            // Check if a key is pressed
            if (e.type == SDL_KEYDOWN)
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (e.key.keysym.scancode == keymap[i])
                    {
                        // Set Vx to the pressed key's value
                        V[x] = i;
                        keyPressed = true;
                        break;
                    }
                }
            }
        }
    }
}