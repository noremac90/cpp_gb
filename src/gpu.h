#pragma once
#include "types.h"
#include "mmu.h"

namespace gb {

class GPU {
    public:
    GPU(MMU& mmu);

    void step(u16 cycles);

    u8 get_tile(u8 x, u8 y);
    void draw_line(u8 line);
    u8 get_color(u8 tile, u8 x, u8 y);
    std::uint32_t palletize(u8 palette, u8 color);


    //private:
    u16 dots;
    MMU& mmu;
    std::unique_ptr<uint32_t[]> frame;
};


}