#include "../include/chip8.h"
#include <SDL2/SDL.h>
#include <iostream>

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int WINDOW_SCALE = 10;

// Forward declarations
void renderGraphics(const Chip8 &chip8, SDL_Renderer *renderer);

int main(int argc, char **argv)
{
    bool debugEnabled = false;

    if (argc < 2 || argc > 3)
    {
        std::cerr << "Usage: " << argv[0] << " <ROM file> [-debug]" << std::endl;
        return 1;
    }

    std::string romFile = argv[1];

    // Check for the -debug flag
    if (argc == 3 && std::string(argv[2]) == "-debug")
    {
        debugEnabled = true;
        std::cout << "Debug mode enabled" << std::endl;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * WINDOW_SCALE,
        SCREEN_HEIGHT * WINDOW_SCALE,
        SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize CHIP-8
    Chip8 chip8(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_SCALE);
    chip8.setDebug(debugEnabled);
    chip8.initialiseFontset();
    if (!chip8.loadROM(argv[1]))
    {
        std::cerr << "Failed to load ROM: " << argv[1] << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event e;

    // Main loop
    while (running)
    {
        // Handle input
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = false;
            }
            else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
            {
                chip8.handleInput(e);
            }
        }

        // Emulate a single cycle
        chip8.emulateCycle();

        // Render graphics if needed
        if (chip8.drawFlag)
        {
            renderGraphics(chip8, renderer);
        }

        // Small delay to control emulation speed
        SDL_Delay(2);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void renderGraphics(const Chip8 &chip8, SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White pixels

    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            if (chip8.gfx[y * SCREEN_WIDTH + x])
            {
                SDL_Rect rect;
                rect.x = x * WINDOW_SCALE;
                rect.y = y * WINDOW_SCALE;
                rect.w = WINDOW_SCALE;
                rect.h = WINDOW_SCALE;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}
