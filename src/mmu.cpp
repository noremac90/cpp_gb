#include "mmu.h"
#include <fstream>
#include <cstring>
#include <string>
#include <fmt/format.h>
#include <SDL2/SDL.h>

namespace gb {
MMU::MMU() :
    vram(new u8[0x2000]),
    hram(new u8[0x7F]) {

    wram[0] = std::make_unique<u8[]>(0x1000);
    wram[1] = std::make_unique<u8[]>(0x1000);

}

void MMU::set(u16 addr, u8 value) {

    if(addr >= 0x2000 && addr <= 0x3FFF) {
        rom_bank = value & 0b11111;
        if(rom_bank % 0x20 == 0)
            rom_bank += 1;
    }

    if(addr >= 0x8000 && addr <= 0x9FFF) {
        vram[addr & 0x3FFF] = value;
    } else if(addr >= 0xC000 && addr <= 0xCFFF) {
        wram[0][addr & 0xFFF] = value;
    } else if(addr >= 0xD000 && addr <= 0xDFFF) {
        wram[1][addr & 0xFFF] = value;
    } else if(addr >= 0xFE00 && addr <= 0xFE9F) {
        std::array<u8, sizeof oam> bytes;
        std::memcpy(bytes.data(), &oam, sizeof oam);
        bytes[addr & 0xFF] = value;
        std::memcpy(&oam, bytes.data(), sizeof oam);
    } else if(addr >= 0xFF00 && addr <= 0xFF7F) {
        if(addr == 0xFF46) {
            
            u16 src_addr = ((u16) value) << 8;
            for(u16 addr = 0x0; addr < 160; addr++) {
                set(0xFE00 + addr, get(src_addr + addr));
            }
        }
        std::array<u8, sizeof io> bytes;
        std::memcpy(bytes.data(), &io, sizeof io);
        bytes[addr & 0x7F] = value;
        std::memcpy(&io, bytes.data(), sizeof io);
    } else if(addr >= 0xFF80 && addr <= 0xFFFE) {
        hram[addr & 0x7F] = value;
    } else if(addr == 0xFFFF) {
        IE = value;
    }
}

u8 MMU::get(u16 addr) {
    if(addr >= 0x0000 && addr <= 0xFF && io.BOOT == 0) {
        return bios[addr & 0xFF];
    } else if(addr >= 0x0000 && addr <= 0x3FFF) {
        return rom[0][addr & 0x3FFF];
    } else if(addr >= 0x4000 && addr <= 0x7FFF) {
        return rom[rom_bank][addr & 0x3FFF];
    } else if(addr >= 0xC000 && addr <= 0xCFFF) {
        return wram[0][addr & 0xFFF];
    } else if(addr >= 0xD000 && addr <= 0xDFFF) {
        return wram[1][addr & 0xFFF];
    } else if(addr >= 0x8000 && addr <= 0x9FFF) {
        return vram[addr - 0x8000];
    } else if(addr >= 0xFF00 && addr <= 0xFF7F) {
        std::array<u8, sizeof io> bytes;
        std::memcpy(bytes.data(), &io, sizeof io);
        return bytes[addr & 0x7F];
    } else if(addr >= 0xFE00 && addr <= 0xFE9F) {
        std::array<u8, sizeof oam> bytes;
        std::memcpy(bytes.data(), &oam, sizeof oam);
        return bytes[addr & 0xFF];
    } else if(addr >= 0xFF80 && addr <= 0xFFFE) {
        return hram[addr & 0x7F];
    } else if(addr == 0xFFFF) {
        return IE;
    }
    return 0xFF;
}

void MMU::load_bios(const std::string_view file) {
    std::ifstream ifs{std::string(file), std::ios::binary};

    ifs.read(reinterpret_cast<char *>(bios.data()), bios.size());

}

void MMU::load_rom(std::string_view file) {
    std::ifstream ifs{std::string(file), std::ios::binary};

    rom[0] = std::make_unique<u8[]>(0x4000);
    rom[1] = std::make_unique<u8[]>(0x4000);
    rom[2] = std::make_unique<u8[]>(0x4000);
    rom[3] = std::make_unique<u8[]>(0x4000);

    ifs.read(reinterpret_cast<char *>(rom[0].get()), 0x4000);
    ifs.read(reinterpret_cast<char *>(rom[1].get()), 0x4000);
    ifs.read(reinterpret_cast<char *>(rom[2].get()), 0x4000);
    ifs.read(reinterpret_cast<char *>(rom[3].get()), 0x4000);
}


}