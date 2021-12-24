#include <cstdio>
#include <cstring>
#include <fmt/format.h>

#include "cpu.h"
#include "mmu.h"

int main() {

    gb::MMU mmu;

    mmu.load_bios("dmg_boot.bin");

    gb::CPU cpu(mmu);

    cpu.dump();


    return 0;
}