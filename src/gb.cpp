#include <cstdio>
#include <cstring>
#include <fmt/format.h>
#include <SDL2/SDL.h>

#include "cpu.h"
#include "mmu.h"

int main() {

    gb::MMU mmu;

    mmu.load_bios("dmg_boot.bin");
    mmu.load_rom("tetris.gb");


    gb::CPU cpu(mmu);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    if(SDL_CreateWindowAndRenderer(512, 512, 0, &window, &renderer) == -1) {
        fmt::print("{}\n", SDL_GetError());
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);

    SDL_Event event;
    Uint32 lastTick;

    lastTick = SDL_GetTicks();


    while (1) {

        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        
        SDL_UpdateTexture(texture, NULL, cpu.gpu.frame.get(), 256 * 4);

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        
        SDL_RenderPresent(renderer);

        Uint32 elapsed = SDL_GetTicks() - lastTick;
        lastTick = SDL_GetTicks();

        uint32_t cycles = 4194304.0f * elapsed / 1000.0f;

        auto end_cycles = cpu.cycles + cycles;

        while(end_cycles > cpu.cycles) {
            cpu.step();
        }
    }


    /*while(true) {
        cpu.dump();
        cpu.step();
    }*/


    return 0;
}