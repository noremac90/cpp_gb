#include <vector>
#include <fmt/format.h>
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

        if(mmu.io.LY == 143) {
            mmu.io.IF |= 1;
        }

    }

    if(mmu.io.LY > 153) {
        mmu.io.LY = 0;
    }

    //mmu.io.LY = 0x90;

}

u8 GPU::get_tile(u8 x, u8 y) {

    u16 base = mmu.io.LCDC & 0b0000'1000 ? 0x9C00 : 0x9800;

    return mmu[base + x / 8 + (y / 8) * 32];
}

u8 GPU::get_color(u8 tile, u8 x, u8 y, bool bg) {

    u8 mask = 1 << (7 - x);

    u16 base = 0x8000;

    if(bg && (mmu.io.LCDC & 0b0001'0000) == 0) {
        if(tile <= 127) {
            base = 0x9000;
        } else {
            base = 0x8800;
        }
    }

    u16 addr = base + tile * 16 + y * 2;

    u8 plane1 = mmu[addr];
    u8 plane2 = mmu[addr + 1];

    u8 bit1 = (plane1 & mask) == mask;
    u8 bit2 = (plane2 & mask) == mask;


    return (bit2 << 1) | bit1;
}

std::uint32_t GPU::palletize(u8 palette, u8 color) {
    std::array<std::uint32_t, 4> colors = { 0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000 };
    //std::array<std::uint32_t, 4> colors = { 0xFF000000, 0xFF555555, 0xFFAAAAAA, 0xFFFFFFFF };

    std::array<u8, 4> conversion{u8(palette& 0x03), u8((palette >> 2) & 0x03), u8((palette >> 4) & 0x03), u8((palette >> 6) & 0x03)};

    return colors[conversion[color]];

}

void GPU::draw_line(u8 y) {

    u8 py = y;

    for(u16 px = 0; px <= 255; px++) {

        u8 y = (py + mmu.io.SCY) % 256;
        u8 x = (px + mmu.io.SCX) % 256;

        u8 tile = get_tile(x, y);

        u8 color = get_color(tile, x % 8, y % 8, true);
        uint32_t pixel = palletize(mmu.io.BGP, color);

        frame[px + py * 256] = pixel;
    }

    std::array<OAM *, 10> sprites = {};
    std::size_t numSprites = 0;

    for(int i = 0; i < mmu.oam.size(); i++) {
        if(mmu.io.LY >= mmu.oam[i].y - 16 && mmu.io.LY <= mmu.oam[i].y + 7 - 16) {
            sprites[numSprites++] = &mmu.oam[i];
        }

        if(numSprites == 10) {
            break;
        }
    }

    for(u16 x = 0; x < 255; x++) {

        OAM *sprite = nullptr;
        for(std::size_t i = 0; i < numSprites; i++) {
            if(x >= sprites[i]->x - 8 && x < sprites[i]->x) {
                sprite = sprites[i];
            }
        }

        if(sprite) {
            u8 palette = sprite->palette == 0 ? mmu.io.OBP0 : mmu.io.OBP1;

            u8 sprite_x = 8 - (sprite->x - x);
            u8 sprite_y = 16 - (sprite->y - mmu.io.LY);

            if(sprite->xflip) {
                sprite_x = 7 - sprite_x;
            }

            if(sprite->yflip) {
                sprite_y = 7 - sprite_y;
            }

            u8 color = get_color(sprite->tile, sprite_x, sprite_y, false);
            if(color != 0) {
                uint32_t pixel = palletize(palette, color);
                frame[x + y * 256] = pixel;
            }
        }
    }

}


}