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
    if(addr >= 0xFF00 && addr <= 0xFF7F) {
        std::array<u8, sizeof io> bytes;
        std::memcpy(bytes.data(), &io, sizeof io);
        bytes[addr & 0x7F] = value;
        std::memcpy(&io, bytes.data(), sizeof io);
    }
}

u8 MMU::get(u16 addr) {
    if(addr >= 0x0000 && addr <= 0xFF && io.BOOT == 0) {
        return bios[addr & 0xFF];
    } else if(addr >= 0xFF00 && addr <= 0xFF7F) {
        std::array<u8, sizeof io> bytes;
        std::memcpy(bytes.data(), &io, sizeof io);
        return bytes[addr & 0x7F];
    }
    return 0xFF;
}

void MMU::load_bios(const std::string_view file) {
    std::ifstream ifs{std::string(file), std::ios::binary};

    ifs.read(reinterpret_cast<char *>(bios.data()), bios.size());

}


}