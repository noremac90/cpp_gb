#include "gpu.h"

namespace gb {

GPU::GPU(MMU& mmu) : dots(0), mmu(mmu), frame(new uint32_t[256 * 256]) {

}

void GPU::step(u16 cycles) {
    dots += cycles;

    if(dots > 456) {
        dots = 0;
        draw_line(mmu.io.LY);
        mmu.io.LY += 1;
    }

    if(mmu.io.LY > 153) {
        mmu.io.LY = 0;
    }

}

u8 GPU::get_tile(u8 x, u8 y) {
    return mmu[0x9800 + x / 8 + (y / 8) * 32];
}

u8 GPU::get_color(u8 tile, u8 x, u8 y) {

    u8 mask = 1 << (7 - x);

    u16 addr = 0x8000 + tile * 16 + y * 2;

    u8 plane1 = mmu[addr];
    u8 plane2 = mmu[addr + 1];

    u8 bit1 = (plane1 & mask) != 0;
    u8 bit2 = (plane2 & mask) != 0;


    return bit1 << 1 | bit2;
}

std::uint32_t GPU::palletize(u8 palette, u8 color) {
    //std::array<std::uint32_t, 4> colors = { 0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000 };
    std::array<std::uint32_t, 4> colors = { 0xFF000000, 0xFF555555, 0xFFAAAAAA, 0xFFFFFFFF };

    std::array<u8, 4> conversion{u8((palette >> 6) & 0x03), u8((palette >> 4) & 0x03), u8((palette >> 2) & 0x03), u8(palette & 0x03)};

    return colors[conversion[color]];

}

void GPU::draw_line(u8 y) {

    

    for(u16 x = 0; x <= 255; x++) {
        u8 tile = get_tile(x, y);
        uint32_t pixel = 0xFFFFFFFF;

        if(tile != 0) {

            u8 color = get_color(tile, x % 8, y % 8);


            if(color != 0) {
                pixel = palletize(mmu.io.BGP, color);
            }
        }

        frame[x + y * 256] = pixel;


    }
}


}