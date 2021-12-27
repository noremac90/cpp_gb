#include <fmt/format.h>
#include "cpu.h"

namespace gb {

CPU::CPU(MMU& mmu) : mmu(mmu) {

}


u8 CPU::fetch8() {
    u8 result = read8(pc++);
    clock();
    return result;
}

u16 CPU::fetch16() {
    return fetch8() | fetch8() << 8;
}

void CPU::write8(u16 addr, u8 value) {
    mmu[addr] = value;
    clock();
}

void CPU::write16(u16 addr, u16 value) {
    write8(addr, value & 0xFF);
    write8(addr + 1, value >> 8);
}

u8 CPU::read8(u16 addr) {
    u8 result = mmu[addr];
    clock();
    return result;
}

u16 CPU::read16(u16 addr) {
    return read8(addr) | read8(addr + 1) << 8;
}

void CPU::clock() {
    cycles += 4;
}


void CPU::step() {
    u8 ins = fetch8();

    switch(ins) {
        case 0x00: break;
        case 0x04: b = alu_inc(b); break;
        case 0x05: b = alu_dec(b); break;
        case 0x06: b = fetch8(); break;
        case 0x08: write16(fetch16(), sp); break;
        case 0x0C: c = alu_inc(c); break;
        case 0x0D: c = alu_dec(c); break;
        case 0x0E: c = fetch8(); break;
        case 0x11: de = fetch16(); break;
        case 0x13: de++; break;
        case 0x15: d = alu_dec(d); break;
        case 0x16: d = fetch8(); break;
        case 0x17: a = bit_rl(a, true); break;
        case 0x18: op_jr(Condition::None, fetch8()); break;
        case 0x1A: a = read8(de); break;
        case 0x1D: e = alu_dec(e); break;
        case 0x1E: e = fetch8(); break;
        case 0x20: op_jr(Condition::NZ, fetch8()); break;
        case 0x21: hl = fetch16(); break;
        case 0x22: write8(hl++, a); break;
        case 0x23: hl++; break;
        case 0x24: h = alu_inc(h); break;
        case 0x28: op_jr(Condition::Z, fetch8()); break;
        case 0x2E: l = fetch8(); break;
        case 0x31: sp = fetch16(); break;
        case 0x32: write8(hl--, a); break;
        case 0x3D: a = alu_dec(a); break;
        case 0x3E: a = fetch8(); break;
        case 0x4F: c = a; break;
        case 0x57: d = a; break;
        case 0x67: h = a; break;
        case 0x77: write8(hl, a); break;
        case 0x78: a = b; break;
        case 0x7B: a = e; break;
        case 0x7C: a = h; break;
        case 0x7D: a = l; break;
        case 0x86: a = alu_add(a, read8(hl)); break;
        case 0x90: a = alu_sub(a, b); break;
        case 0xAF: a = alu_xor(a, a); break;
        case 0xBE: alu_sub(a, read8(hl)); break;
        case 0xC1: bc = pop(); break;
        case 0xC5: clock(); push(bc); break;
        case 0xC9: op_ret(Condition::None); break;
        case 0xCB: op_cb(); break;
        case 0xCD: op_call(Condition::None, fetch16()); break;
        case 0xE0: write8(0xFF00 + fetch8(), a); break;
        case 0xE2: write8(0xFF00 + c, a); break;
        case 0xEA: write8(fetch16(), a); break;
        case 0xF0: a = read8(0xFF00 + fetch8()); break;
        case 0xFE: alu_sub(a, fetch8()); break;
        default: fmt::print("Unknown opcode {:02X} at {:04X}", ins, pc - 1); exit(0); break;
    }

}

void CPU::op_cb() {
    u8 ins = fetch8();
    u8 x = ins >> 6;
    u8 y = (ins >> 3) & 0b111;
    u8 z = ins & 0b111;

    decltype(&CPU::bit_rl) fn[] = { nullptr, nullptr, &CPU::bit_rl, nullptr, nullptr, nullptr, nullptr, nullptr };

    if(x == 0) {
        switch(z) {
            case 0: b = (this->*fn[y])(b, false); break;
            case 1: c = (this->*fn[y])(c, false); break;
            case 2: d = (this->*fn[y])(d, false); break;
            case 3: e = (this->*fn[y])(e, false); break;
            case 4: h = (this->*fn[y])(h, false); break;
            case 5: write8(hl, (this->*fn[y])(read8(hl), false)); break;
            case 6: a = (this->*fn[y])(a, false); break;
        }

    } else if(x == 1) {
        switch(z) {
            case 0: bit_test(b, y); break;
            case 1: bit_test(c, y); break;
            case 2: bit_test(d, y); break;
            case 3: bit_test(e, y); break;
            case 4: bit_test(h, y); break;
            case 5: bit_test(read8(hl), y); break;
            case 6: bit_test(a, y); break;
        }
    }

    //fmt::print("x: {}, y: {}, z: {}", x, y, z);

}

void CPU::push(u16 value) {
    write8(--sp, value >> 8);
    write8(--sp, value & 0xFF);
}

u16 CPU::pop() {
    u8 l = read8(sp++);
    u8 h = read8(sp++);
    return h << 8 | l;
}

void CPU::op_call(Condition condition, u16 addr) {
    if(condition == Condition::None
    || condition == Condition::C && f.c
    || condition == Condition::NC && !f.c
    || condition == Condition::Z && f.z
    || condition == Condition::NZ && !f.z
    ) {
        clock();
        push(pc);
        pc = addr;
    }
}

void CPU::op_ret(Condition condition) {
    if(condition == Condition::None
    || condition == Condition::C && f.c
    || condition == Condition::NC && !f.c
    || condition == Condition::Z && f.z
    || condition == Condition::NZ && !f.z
    ) {        
        u16 addr = pop();
        pc = addr;
        clock(); 
    }
}

void CPU::op_jr(Condition condition, i8 offset) {
    u16 addr = pc + offset;

    if(condition == Condition::None
    || condition == Condition::C && f.c
    || condition == Condition::NC && !f.c
    || condition == Condition::Z && f.z
    || condition == Condition::NZ && !f.z
    ) {
        pc = addr;
        clock();
    }
}

u8 CPU::bit_test(u8 value, u8 bit) {
    f.z = ~value & 1 << bit;
    f.n = false;
    f.h = true;
    return value;
}

u8 CPU::bit_rl(u8 value, bool accum) {
    bool carry_out = value & 0b1000'0000;

    u8 result = (value << 1) | f.c;

    f.z = accum ? false : result == 0;
    f.h = false;
    f.n = false;
    f.c = carry_out;

    return result;
}

void CPU::dump() {
    std::array<u8, 4> nextBytes{ mmu[pc], mmu[pc + 1], mmu[pc + 2], mmu[pc + 3] };
    fmt::print("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} {:02X}) Z: {} N: {} H: {} C: {}\n",
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
        nextBytes[3],
        (u8) f.z,
        (u8) f.n,
        (u8) f.h,
        (u8) f.c
        );
}

u8 CPU::alu_xor(u8 lhs, u8 rhs) {
    u8 result = lhs ^ rhs;
    f.c = false;
    f.h = false;
    f.n = false;
    f.z = result == 0;
    return result;
}

u8 CPU::alu_inc(u8 value) {
    u8 result = value + 1;
    f.z = result == 0;
    f.n = false;
    f.h = (value & 0xf) + 1 & 0x10;

    return result;

}

u8 CPU::alu_dec(u8 value) {
    u8 result = value - 1;
    f.z = result == 0;
    f.n = true;
    f.h = (value & 0xf) - 1 & 0x10;

    return result;

}

u8 CPU::alu_sub(u8 lhs, u8 rhs) {
    u8 result = lhs - rhs;

    f.z = result == 0;
    f.n = true;
    f.h = (lhs & 0xf) - (rhs & 0xf) & 0x10;
    f.c = rhs > lhs;

    return result;
}

u8 CPU::alu_add(u8 lhs, u8 rhs) {
    u8 result;

    bool carry = __builtin_add_overflow(lhs, rhs, &result);

    f.z = result == 0;
    f.n = false;
    f.h = (lhs & 0xf) + (rhs & 0xf) & 0x10;
    f.c = carry;

    return result;
}

}