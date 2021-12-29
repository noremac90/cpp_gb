#include <fmt/format.h>
#include <SDL2/SDL.h>
#include "cpu.h"

namespace gb {

CPU::CPU(MMU& mmu) : mmu(mmu), gpu(mmu) {

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
    if(addr >= 0x8000 && addr <= 0x9FFF) {
        //fmt::print("Write {:02X} to {:04X}\n", value, addr);
    }
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
    gpu.step(4);

    static int DIVCnt = 0;

    DIVCnt += 4;

    if(DIVCnt > 256) {
        mmu.io.DIVA += 1;
        DIVCnt = 0;
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
            
    mmu.io.JOYP |= 0b0000'1111;

    if((mmu.io.JOYP & 0b0010'0000) == 0) {
        if(state[SDL_SCANCODE_Z])
            mmu.io.JOYP &= ~1;
        if(state[SDL_SCANCODE_X])
            mmu.io.JOYP &= ~2;
        if(state[SDL_SCANCODE_BACKSPACE])
            mmu.io.JOYP &= ~4;
        if(state[SDL_SCANCODE_RETURN])
            mmu.io.JOYP &= ~8;
    } else if((mmu.io.JOYP & 0b0001'0000) == 0) {
        // R L U D
        if(state[SDL_SCANCODE_RIGHT])
            mmu.io.JOYP &= ~1;
        if(state[SDL_SCANCODE_LEFT])
            mmu.io.JOYP &= ~2;
        if(state[SDL_SCANCODE_UP])
            mmu.io.JOYP &= ~4;
        if(state[SDL_SCANCODE_DOWN])
            mmu.io.JOYP &= ~8;
    }
    
}

void CPU::check_int() {
    /*if(pc == 0xC2BE || pc == 0xC2C0) {
        fmt::print("IE: {:08b} IF: {:08b} ime: {}\n", mmu.IE, mmu.io.IF, ime);
    }*/
    if(ime) {
        if(mmu.IE & mmu.io.IF) {
            u8 mask = mmu.io.IF & (-mmu.io.IF);
            // 00001 = 1 = 0x40 = 64 = 0
            // 00010 = 2 = 0x48 = 72 = 8
            // 00100 = 4 = 0x50 = 80 = 16
            // 01000 = 8 = 0x58 = 88 = 24
            // 10000 = 16 = 0x60 = 96 = 32
            u16 rst;
            //fmt::print("IE: {:08b} IF: {:08b} ime: {}\n", mmu.IE, mmu.io.IF, ime);
            //fmt::print("mask {:08b}\n", mask);
            if(mask & 1) {
                //
                //fmt::print("Vsync 0x40\n");
                rst = 0x40;
            } else if(mask & 2) {
                rst = 0x48;
            } else if(mask & 4) {
                rst = 0x50;
            } else if(mask & 8) {
                rst = 0x58;
            } else if(mask & 16) {
                rst = 0x60;
            }

            ime = false;
            mmu.io.IF &= ~mask;

            op_rst(rst);
            //fmt::print("IE: {:08b} IF: {:08b} ime: {}\n", mmu.IE, mmu.io.IF, ime);
        }

    }
}

void CPU::step() {

    //mmu.io.JOYP = 0b0001111;
    check_int();

    u8 ins = fetch8();

    switch(ins) {
        case 0x00: break;
        case 0x01: bc = fetch16(); break;
        case 0x02: write8(bc, a); break;
        case 0x03: ++bc; break;
        case 0x04: b = alu_inc(b); break;
        case 0x05: b = alu_dec(b); break;
        case 0x06: b = fetch8(); break;
        case 0x07: a = bit_rlc(a, true); break;

        case 0x08: write16(fetch16(), sp); break;
        case 0x09: hl = alu_add16(hl, bc); break;
        case 0x0A: a = read8(bc); break;
        case 0x0B: --bc; break;
        case 0x0C: c = alu_inc(c); break;
        case 0x0D: c = alu_dec(c); break;
        case 0x0E: c = fetch8(); break;
        case 0x0F: a = bit_rrc(a, true); break;

        case 0x11: de = fetch16(); break;
        case 0x12: write8(de, a); break;
        case 0x13: ++de; break;
        case 0x14: d = alu_inc(d); break;
        case 0x15: d = alu_dec(d); break;
        case 0x16: d = fetch8(); break;
        case 0x17: a = bit_rl(a, true); break;

        case 0x18: op_jr(Condition::None, fetch8()); break;
        case 0x19: hl = alu_add16(hl, de); break;
        case 0x1A: a = read8(de); break;
        case 0x1B: --de; break;
        case 0x1C: e = alu_inc(e); break;
        case 0x1D: e = alu_dec(e); break;
        case 0x1E: e = fetch8(); break;
        case 0x1F: a = bit_rr(a, true); break;

        case 0x20: op_jr(Condition::NZ, fetch8()); break;
        case 0x21: hl = fetch16(); break;
        case 0x22: write8(hl++, a); break;
        case 0x23: ++hl; break;
        case 0x24: h = alu_inc(h); break;
        case 0x25: h = alu_dec(h); break;
        case 0x26: h = fetch8(); break;
        case 0x27: break;

        case 0x28: op_jr(Condition::Z, fetch8()); break;
        case 0x29: hl = alu_add16(hl, hl); break;
        case 0x2A: a = read8(hl++); break;
        case 0x2B: --hl; break;
        case 0x2C: l = alu_inc(l); break;
        case 0x2D: l = alu_dec(l); break;
        case 0x2E: l = fetch8(); break;
        case 0x2F: a = ~a; f.n = true; f.h = true; break;

        case 0x30: op_jr(Condition::NC, fetch8()); break;
        case 0x31: sp = fetch16(); break;
        case 0x32: write8(hl--, a); break;
        case 0x33: ++sp; break;
        case 0x34: write8(hl, alu_inc(read8(hl))); break;
        case 0x35: write8(hl, alu_dec(read8(hl))); break;
        case 0x36: write8(hl, fetch8()); break;
        case 0x37: f.n = false; f.h = false; f.c = true; break;

        case 0x38: op_jr(Condition::C, fetch8()); break;
        case 0x39: hl = alu_add16(hl, sp); break;
        case 0x3A: a = read8(hl--); break;
        case 0x3B: --sp; break;
        case 0x3C: a = alu_inc(a); break;
        case 0x3D: a = alu_dec(a); break;
        case 0x3E: a = fetch8(); break;
        case 0x3F: f.n = false; f.h = false; f.c = !f.c; break;

        case 0x40: break; // a = a
        case 0x41: b = c; break;
        case 0x42: b = d; break;
        case 0x43: b = e; break;
        case 0x44: b = h; break;
        case 0x45: b = l; break;
        case 0x46: b = read8(hl); break;
        case 0x47: b = a; break;

        case 0x48: c = b; break;
        case 0x49: break; // c = c
        case 0x4A: c = d; break;
        case 0x4B: c = e; break;
        case 0x4C: c = h; break;
        case 0x4D: c = l; break;
        case 0x4E: c = read8(hl); break;
        case 0x4F: c = a; break;

        case 0x50: d = b; break;
        case 0x51: d = c; break;
        case 0x52: break; // d = d
        case 0x53: d = e; break;
        case 0x54: d = h; break;
        case 0x55: d = l; break;
        case 0x56: d = read8(hl); break;
        case 0x57: d = a; break;

        case 0x58: e = b; break;
        case 0x59: e = c; break;
        case 0x5A: e = d; break;
        case 0x5B: break; // e = e
        case 0x5C: e = h; break;
        case 0x5D: e = l; break;
        case 0x5E: e = read8(hl); break;
        case 0x5F: e = a; break;

        case 0x60: h = b; break;
        case 0x61: h = c; break;
        case 0x62: h = d; break;
        case 0x63: h = e; break;
        case 0x64: break; // h = h
        case 0x65: h = l; break;
        case 0x66: h = read8(hl); break;
        case 0x67: h = a; break;

        case 0x68: l = b; break;
        case 0x69: l = c; break;
        case 0x6A: l = d; break;
        case 0x6B: l = e; break;
        case 0x6C: l = h; break;
        case 0x6D: break; // l = l
        case 0x6E: l = read8(hl); break;
        case 0x6F: l = a; break;

        case 0x70: write8(hl, b); break;
        case 0x71: write8(hl, c); break;
        case 0x72: write8(hl, d); break;
        case 0x73: write8(hl, e); break;
        case 0x74: write8(hl, h); break;
        case 0x75: write8(hl, l); break;
        
        case 0x76: break;

        case 0x77: write8(hl, a); break;

        case 0x78: a = b; break;
        case 0x79: a = c; break;
        case 0x7A: a = d; break;
        case 0x7B: a = e; break;
        case 0x7C: a = h; break;
        case 0x7D: a = l; break;
        case 0x7E: a = read8(hl); break;
        case 0x7F: break; // a = a

        case 0x80: a = alu_add(a, b, false); break;
        case 0x81: a = alu_add(a, c, false); break;
        case 0x82: a = alu_add(a, d, false); break;
        case 0x83: a = alu_add(a, e, false); break;
        case 0x84: a = alu_add(a, h, false); break;
        case 0x85: a = alu_add(a, l, false); break;
        case 0x86: a = alu_add(a, read8(hl), false); break;
        case 0x87: a = alu_add(a, a, false); break;

        case 0x88: a = alu_add(a, b, true); break;
        case 0x89: a = alu_add(a, c, true); break;
        case 0x8A: a = alu_add(a, d, true); break;
        case 0x8B: a = alu_add(a, e, true); break;
        case 0x8C: a = alu_add(a, h, true); break;
        case 0x8D: a = alu_add(a, l, true); break;
        case 0x8E: a = alu_add(a, read8(hl), true); break;
        case 0x8F: a = alu_add(a, a, true); break;

        case 0x90: a = alu_sub(a, b, false); break;
        case 0x91: a = alu_sub(a, c, false); break;
        case 0x92: a = alu_sub(a, d, false); break;
        case 0x93: a = alu_sub(a, e, false); break;
        case 0x94: a = alu_sub(a, h, false); break;
        case 0x95: a = alu_sub(a, l, false); break;
        case 0x96: a = alu_sub(a, read8(hl), false); break;
        case 0x97: a = alu_sub(a, a, false); break;

        case 0x98: a = alu_sub(a, b, true); break;
        case 0x99: a = alu_sub(a, c, true); break;
        case 0x9A: a = alu_sub(a, d, true); break;
        case 0x9B: a = alu_sub(a, e, true); break;
        case 0x9C: a = alu_sub(a, h, true); break;
        case 0x9D: a = alu_sub(a, l, true); break;
        case 0x9E: a = alu_sub(a, read8(hl), true); break;
        case 0x9F: a = alu_sub(a, a, true); break;
        
        case 0xA0: a = alu_and(a, b); break;
        case 0xA1: a = alu_and(a, c); break;
        case 0xA2: a = alu_and(a, d); break;
        case 0xA3: a = alu_and(a, e); break;
        case 0xA4: a = alu_and(a, h); break;
        case 0xA5: a = alu_and(a, l); break;
        case 0xA6: a = alu_and(a, read8(hl)); break;
        case 0xA7: a = alu_and(a, a); break;

        case 0xA8: a = alu_xor(a, b); break;
        case 0xA9: a = alu_xor(a, c); break;
        case 0xAA: a = alu_xor(a, d); break;
        case 0xAB: a = alu_xor(a, e); break;
        case 0xAC: a = alu_xor(a, h); break;
        case 0xAD: a = alu_xor(a, l); break;
        case 0xAE: a = alu_xor(a, read8(hl)); break;
        case 0xAF: a = alu_xor(a, a); break;
        
        case 0xB0: a = alu_or(a, b); break;
        case 0xB1: a = alu_or(a, c); break;
        case 0xB2: a = alu_or(a, d); break;
        case 0xB3: a = alu_or(a, e); break;
        case 0xB4: a = alu_or(a, h); break;
        case 0xB5: a = alu_or(a, l); break;
        case 0xB6: a = alu_or(a, read8(hl)); break;
        case 0xB7: a = alu_or(a, a); break;
        
        case 0xB8: alu_sub(a, b, false); break;
        case 0xB9: alu_sub(a, c, false); break;
        case 0xBA: alu_sub(a, d, false); break;
        case 0xBB: alu_sub(a, e, false); break;
        case 0xBC: alu_sub(a, h, false); break;
        case 0xBD: alu_sub(a, l, false); break;
        case 0xBE: alu_sub(a, read8(hl), false); break;
        case 0xBF: alu_sub(a, a, false); break;
        
        case 0xC0: op_ret(Condition::NZ); break;
        case 0xC1: bc = pop(); break;
        case 0xC2: op_jump(Condition::NZ, fetch16()); break;
        case 0xC3: op_jump(Condition::None, fetch16()); break;
        case 0xC4: op_call(Condition::NZ, fetch16()); break;
        case 0xC5: clock(); push(bc); break;
        case 0xC6: a = alu_add(a, fetch8(), false); break;
        case 0xC7: op_rst(0x00); break;

        case 0xC8: op_ret(Condition::Z); break;
        case 0xC9: op_ret(Condition::None); break;
        case 0xCA: op_jump(Condition::Z, fetch16()); break;
        case 0xCB: op_cb(); break;
        case 0xCC: op_call(Condition::Z, fetch16()); break;
        case 0xCD: op_call(Condition::None, fetch16()); break;
        case 0xCE: a = alu_add(a, fetch8(), true); break;
        case 0xCF: op_rst(0x08); break;

        case 0xD0: op_ret(Condition::NZ); break;
        case 0xD1: de = pop(); break;
        case 0xD2: op_jump(Condition::NC, fetch16()); break;
        // D3
        case 0xD4: op_call(Condition::NC, fetch16()); break;
        case 0xD5: clock(); push(de); break;
        case 0xD6: a = alu_sub(a, fetch8(), false); break;
        case 0xD7: op_rst(0x10); break;

        case 0xD8: op_ret(Condition::C); break;
        case 0xD9: ime = true; pc = pop(); clock(); break;
        case 0xDA: op_jump(Condition::C, fetch16()); break;
        // DB
        case 0xDC: op_call(Condition::C, fetch16()); break;
        // DD
        case 0xDE: a = alu_sub(a, fetch8(), true); break;
        case 0xDF: op_rst(0x18); break;

        case 0xE0: write8(0xFF00 + fetch8(), a); break;
        case 0xE1: hl = pop(); break;
        case 0xE2: write8(0xFF00 + c, a); break;
        // E3
        // E4
        case 0xE5: clock(); push(hl); break;
        case 0xE6: a = alu_and(a, fetch8()); break;
        case 0xE7: op_rst(0x20); break;

        case 0xE8: sp += (i8) fetch8(); break;
        case 0xE9: pc = hl; break;
        case 0xEA: write8(fetch16(), a); break;
        // EB
        // EC
        // ED
        case 0xEE: a = alu_xor(a, fetch8()); break;
        case 0xEF: op_rst(0x28); break;

        case 0xF0: a = read8(0xFF00 + fetch8()); break;
        case 0xF1: af = pop(); break;
        case 0xF2: a = read8(0xFF00 + c); break;
        case 0xF3: ime = false; break;
        // F4
        case 0xF5: clock(); push(af); break;
        case 0xF6: a = alu_or(a, fetch8()); break;
        case 0xF7: op_rst(0x30); break;
        case 0xF8: hl = sp + (i8) fetch8(); break;
        case 0xF9: sp = hl; break;

        case 0xFA: a = read8(fetch16()); break;
        case 0xFB: ime = true; break;
        // FC
        // FD
        case 0xFE: alu_sub(a, fetch8(), false); break;
        case 0xFF: op_rst(0x38); break;
        default: fmt::print("Unknown opcode {:02X} at {:04X}", ins, pc - 1); exit(0); break;
    }

    /*if(pc == 0xC2BE || pc == 0xC2C0) {
        fmt::print("IE: {:08b} IF: {:08b} ime: {}\n", mmu.IE, mmu.io.IF, ime);
    }*/

}

void CPU::op_cb() {
    u8 ins = fetch8();
    u8 x = ins >> 6;
    u8 y = (ins >> 3) & 0b111;
    u8 z = ins & 0b111;

    decltype(&CPU::bit_rl) fn[] = { &CPU::bit_rlc, &CPU::bit_rrc, &CPU::bit_rl, &CPU::bit_rr, &CPU::bit_sla, &CPU::bit_sra, &CPU::bit_swap, &CPU::bit_srl };

    if(x == 0) {
        switch(z) {
            case 0: b = (this->*fn[y])(b, false); break;
            case 1: c = (this->*fn[y])(c, false); break;
            case 2: d = (this->*fn[y])(d, false); break;
            case 3: e = (this->*fn[y])(e, false); break;
            case 4: h = (this->*fn[y])(h, false); break;
            case 5: l = (this->*fn[y])(l, false); break;
            case 6: write8(hl, (this->*fn[y])(read8(hl), false)); break;
            case 7: a = (this->*fn[y])(a, false); break;
        }

    } else if(x == 1) {
        switch(z) {
            case 0: bit_test(b, y); break;
            case 1: bit_test(c, y); break;
            case 2: bit_test(d, y); break;
            case 3: bit_test(e, y); break;
            case 4: bit_test(h, y); break;
            case 5: bit_test(l, y); break;
            case 6: bit_test(read8(hl), y); break;
            case 7: bit_test(a, y); break;
        }
    } else if(x == 2) {
        switch(z) {
            case 0: b = bit_reset(b, y); break;
            case 1: c = bit_reset(c, y); break;
            case 2: d = bit_reset(d, y); break;
            case 3: e = bit_reset(e, y); break;
            case 4: h = bit_reset(h, y); break;
            case 5: l = bit_reset(l, y); break;
            case 6: write8(hl, bit_reset(read8(hl), y)); break;
            case 7: a = bit_reset(a, y); break;
        }
    } else if(x == 3) {
        switch(z) {
            case 0: b = bit_set(b, y); break;
            case 1: c = bit_set(c, y); break;
            case 2: d = bit_set(d, y); break;
            case 3: e = bit_set(e, y); break;
            case 4: h = bit_set(h, y); break;
            case 5: l = bit_set(l, y); break;
            case 6: write8(hl, bit_set(read8(hl), y)); break;
            case 7: a = bit_set(a, y); break;
        }
    } else {
        fmt::print("Unkown CB opcode x: {}, y: {}, z: {}", x, y, z);
        exit(0);
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

void CPU::op_rst(u16 addr) {
    clock();
    push(pc);
    pc = addr;
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

void CPU::op_jump(Condition condition, u16 addr) {
    if(condition == Condition::None
    || condition == Condition::C && f.c
    || condition == Condition::NC && !f.c
    || condition == Condition::Z && f.z
    || condition == Condition::NZ && !f.z
    ) {
        clock();
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

u8 CPU::bit_rlc(u8 value, bool accum) {
    bool carry_out = value & 0b1000'0000;

    u8 result = (value << 1) | carry_out;

    f.z = accum ? false : result == 0;
    f.h = false;
    f.n = false;
    f.c = carry_out;

    return result;
}

u8 CPU::bit_rrc(u8 value, bool accum) {
    bool carry_out = value & 0b00000001;

    u8 result = (value >> 1) | (carry_out << 7);

    f.z = accum ? false : result == 0;
    f.h = false;
    f.n = false;
    f.c = carry_out;

    return result;
}

u8 CPU::bit_rr(u8 value, bool accum) {
    bool carry_out = value & 0b00000001;

    u8 result = (value >> 1) | (f.c << 7);

    f.z = accum ? false : result == 0;
    f.h = false;
    f.n = false;
    f.c = carry_out;

    return result;
}

u8 CPU::bit_srl(u8 value, bool) {
    bool carry_out = value & 0b00000001;

    u8 result = value >> 1;

    f.z = result == 0;
    f.h = 0;
    f.n = 0;
    f.c = carry_out;

    return result;
}

u8 CPU::bit_sra(u8 value, bool) {
    bool carry_out = value & 0b00000001;

    u8 result = value >> 1;

    result = value & 0b1000'0000;

    f.z = result == 0;
    f.h = 0;
    f.n = 0;
    f.c = carry_out;

    return result;
}

u8 CPU::bit_sla(u8 value, bool) {
    bool carry_out = value & 0b1000'0000;

    u8 result = value << 1;

    f.z = result == 0;
    f.h = 0;
    f.n = 0;
    f.c = carry_out;

    return result;
}

u8 CPU::bit_swap(u8 value, bool) {
    

    u8 result = (value >> 4) | (value << 4);

    f.z = result == 0;
    f.h = false;
    f.n = false;
    f.c = false;

    return result;
}

u8 CPU::bit_set(u8 value, u8 bit) {
    return value | (1 << bit);
}

u8 CPU::bit_reset(u8 value, u8 bit) {
    return value & ~(1 << bit);
}

void CPU::dump() {
    std::array<u8, 4> nextBytes{ mmu[pc], mmu[pc + 1], mmu[pc + 2], mmu[pc + 3] };
    fmt::print("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} {:02X}) Z: {} N: {} H: {} C: {} LY: {}\n",
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
        (u8) f.c,
        mmu.io.LY
        );
}

void CPU::dump_std() {
    std::array<u8, 4> nextBytes{ mmu[pc], mmu[pc + 1], mmu[pc + 2], mmu[pc + 3] };
    fmt::print("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} {:02X})\n",
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

u8 CPU::alu_sub(u8 lhs, u8 rhs, bool carry_in) {
    u8 result;

    bool carry = __builtin_sub_overflow(lhs, rhs, &result);

    if(carry_in) {
       carry |= __builtin_sub_overflow(result, (u8) f.c, &result);
    }


    f.z = result == 0;
    f.n = true;
    f.h = (lhs & 0xf) - (rhs & 0xf) - (carry_in ? f.c : 0) & 0x10;
    f.c = carry;

    return result;
}

u8 CPU::alu_add(u8 lhs, u8 rhs, bool carry_in) {
    u8 result;

    bool carry = __builtin_add_overflow(lhs, rhs, &result);

    if(carry_in) {
        carry |= __builtin_add_overflow(result, (u8) f.c, &result);
    }

    f.z = result == 0;
    f.n = false;
    f.h = (lhs & 0xf) + (rhs & 0xf) + (carry_in ? f.c : 0) & 0x10;
    f.c = carry;

    return result;
}

u8 CPU::alu_or(u8 lhs, u8 rhs) {
    u8 result = lhs | rhs;

    f.z = result == 0;
    f.n = false;
    f.h = false;
    f.c = false;

    return result;
}

u8 CPU::alu_and(u8 lhs, u8 rhs) {
    u8 result = lhs & rhs;

    f.z = result == 0;
    f.n = false;
    f.h = true;
    f.c = false;

    return result;
}


u16 CPU::alu_add16(u16 lhs, u16 rhs) {
    u16 result;

    bool carry = __builtin_add_overflow(lhs, rhs, &result);

    f.c = carry;    
    f.n = false;
    f.h = (lhs & 0xFFF) + (rhs & 0xFFF) & 0x1000;

    return result;
}

}