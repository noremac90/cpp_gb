#include <cstdio>
#include <cstring>
#include <fmt/format.h>

#include "cpu.h"
#include "mmu.h"

int main() {

    gb::MMU mmu;

    mmu.load_bios("dmg_boot.bin");

    gb::CPU cpu(mmu);

    fmt::print("{:02X}", mmu[0]);


    return 0;
}