#pragma once
#include <array>
#include <memory>
#include <string_view>
#include "types.h"
namespace gb {

struct IO {
    u8 JOYP;
    u8 SB;
    u8 SC;
    u8 pad7;
    u8 DIVA;
    u8 TIM;
    u8 TMA;
    u8 TAC;
    u8 pad1[8];
    u8 IF;
    u8 NR10;
    u8 NR11;
    u8 NR12;
    u8 NR14;
    u8 pad2;
    u8 NR21;
    u8 NR22;
    u8 NR23;
    u8 NR24;
    u8 NR30;
    u8 NR31;
    u8 NR32;
    u8 NR33;
    u8 NR34;
    u8 pad3;
    u8 NR41;
    u8 NR42;
    u8 NR43;
    u8 NR44;
    u8 NR50;
    u8 NR51;
    u8 NR52;
    u8 pad4[9];
    u8 WAVE[0x10];
    u8 LCDC;
    u8 STAT;
    u8 SCY;
    u8 SCX;
    u8 LY;
    u8 LYC;
    u8 DMA;
    u8 BGP;
    u8 OBP0;
    u8 OBP1;
    u8 WY;
    u8 WX;
    u8 pad5[4];
    u8 BOOT;
    u8 pad6[0x2F];
} __attribute__((packed));

struct OAM {
    u8 y;
    u8 x;
    u8 tile;
    union {
        u8: 4;
        u8 palette: 1;
        bool xflip: 1;
        bool yflip: 1;
        u8 priority: 1;
    };
} __attribute__((packed));


class MMU {
public:
    class MemRef {
        public:
        MemRef& operator=(u8 value) {
            mmu.set(addr, value);
            return *this;
        }

        operator u8() {
            return mmu.get(addr);
        }
        
        private:
        MemRef(MMU& mmu, u16 addr) : mmu(mmu), addr(addr) {

        }

        friend class MMU;

        private:
        MMU& mmu;
        u16 addr;
    };



    MMU();

    void set(u16 addr, u8 value);

    u8 get(u16 addr);

    MemRef operator[](u16 addr) {
        return MemRef{*this, addr};
    }

    void load_bios(std::string_view file);

    void load_rom(std::string_view file);

    std::array<OAM, 40> oam = {};
    struct IO io = {};
    u8 IE = 0;

private:
    std::array<u8, 256> bios; // 0x0000-0x00FF
    std::array<std::unique_ptr<u8[]>, 256> rom; // Rom banks 0x0000-0x7FFF

    std::unique_ptr<u8[]> vram; // 0x8000-0x9FFF

    std::unique_ptr<u8[]> hram; // 0xFF80-0xFFFE
};

}