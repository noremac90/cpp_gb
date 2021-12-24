#include <fmt/format.h>
#include "cpu.h"

namespace gb {

CPU::CPU(MMU& mmu) : mmu(mmu) {

}

void CPU::dump() {
    std::array<u8, 4> nextBytes{ mmu[pc], mmu[pc + 1], mmu[pc + 2], mmu[pc + 3] };
    fmt::print("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} {:02X})",
        a,
        (u8) f,
        b,
        c,
        d,
        e,
        h,
        l,
        sp,
        pc,
        nextBytes[0],
        nextBytes[1],
        nextBytes[2],
        nextBytes[3]);
}

}