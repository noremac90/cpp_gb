#include "mmu.h"
#include <fstream>
#include <cstring>
#include <string>

namespace gb {
MMU::MMU() :
    vram(new u8[0x2000]),
    hram(new u8[0x7F]) {

}

void MMU::set(u16 addr, u8 value) {
    if(addr >= 0x8000 && addr <= 0x9FFF) {
        vram[addr & 0x3FFF] = value;
    } else if(addr >= 0xFFE0 && addr <= 0xFE9F) {
        std::array<u8, sizeof oam> bytes;
        std::memcpy(bytes.data(), &oam, sizeof oam);
        bytes[addr & 0xFF] = value;
        std::memcpy(&oam, bytes.data(), sizeof oam);
    } else if(addr >= 0xFF00 && addr <= 0xFF7F) {
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
    } else if(addr >= 0x0000 && addr <= 0x3FFF) {
        return rom[1][addr & 0x3FFF];
    } else if(addr >= 0x8000 && addr <= 0x9FFF) {
        return vram[addr & 0x3FFF];
    } else if(addr >= 0xFF00 && addr <= 0xFF7F) {
        std::array<u8, sizeof io> bytes;
        std::memcpy(bytes.data(), &io, sizeof io);
        return bytes[addr & 0x7F];
    } else if(addr >= 0xFFE0 && addr <= 0xFE9F) {
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

    ifs.read(reinterpret_cast<char *>(rom[0].get()), 0x4000);
    ifs.read(reinterpret_cast<char *>(rom[1].get()), 0x4000);
}


}