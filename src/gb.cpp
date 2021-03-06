#include <cstdio>
#include <cstring>
#include <fmt/format.h>
#include <SDL2/SDL.h>
#include <fstream>

#include "cpu.h"
#include "mmu.h"

int main(int argc, char *argv[]) {

    gb::MMU mmu;

    mmu.load_bios("dmg_boot.bin");
    mmu.load_rom("mario.gb");

    

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
    bool bp = false;


    while (1) {



        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;

        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_A]) {
            
            bp = true;
        }

        if (state[SDL_SCANCODE_ESCAPE]) {
            
            break;
        }

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
            if(bp) {
                cpu.dump_std();
            }
            cpu.step();
        }
    }


    /*while(true) {
        mmu.io.JOYP = 0b0000111;
        if(mmu.io.BOOT == 1) {
            cpu.dump();
        }
        cpu.step();
    }*/


    return 0;
}