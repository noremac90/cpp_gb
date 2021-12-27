#include <cstdio>
#include <cstring>
#include <fmt/format.h>

#include "cpu.h"
#include "mmu.h"

int main() {

    gb::MMU mmu;

    mmu.load_bios("dmg_boot.bin");
    mmu.load_rom("tetris.gb");

    mmu.io.LY = 0x90;

    gb::CPU cpu(mmu);

    while(true) {
        cpu.dump();
        cpu.step();
    }


    return 0;
}