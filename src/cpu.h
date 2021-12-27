#pragma once
#include "mmu.h"
#include "types.h"

namespace gb {

template<typename High, typename Low>
class Register16 {
public:
    Register16(High& high, Low& low) : high(high), low(low) { }

    u16 get() {
        return high << 8 | low;
    }

    void set(u16 value) {
        high = value >> 8;
        low = value & 0xFF;
    }

    Register16& operator=(Register16& rhs) {
        set(rhs);
        return *this;
    }

    Register16& operator=(u16 rhs) {
        set(rhs);
        return *this;
    }

    Register16& operator+=(u16 rhs) {
        set(get() + rhs);
        return *this;
    }

    Register16& operator-=(u16 rhs) {
        set(get() - rhs);
        return *this;
    }

    u16 operator++(int dummy) {
        u16 tmp = *this;
        *this += 1;
        return tmp;
    }

    u16 operator--(int dummy) {
        u16 tmp = *this;
        *this -= 1;
        return tmp;
    }

    u16 operator++() {
        *this += 1;
        return *this;
    }

    u16 operator--() {
        *this -= 1;
        return *this;
    }

    operator u16() {
        return get();
    }

private:
    High& high;
    Low& low;
};

template<u8 bit>
class Bit {
public:
    Bit(u8& src) : src(src) {

    }

    Bit<bit>& operator=(bool value) {
        if(value) {
            src |= 1 << bit;
        } else {
            src &= ~(1 << bit);
        }
        return *this;
    }

    operator bool() {
        return src & 1 << bit;
    }
private:
    u8& src;
};

class Flags {
public:
    Flags(u8 value) : value(value & 0b1111'0000) {
    }

    operator u8() {
        return value;
    };



    Bit<7> z{value};
    Bit<6> n{value};
    Bit<5> h{value};
    Bit<4> c{value};

private:
    u8 value;
};



class CPU {

    enum class Condition {
        None,
        Z,
        NZ,
        C,
        NC
    };

    public:
    CPU(MMU& mmu);

    u8 fetch8();
    u16 fetch16();

    void write8(u16 addr, u8 value);
    void write16(u16 addr, u16 value);

    u8 read8(u16 addr);
    u16 read16(u16 addr);

    void clock();

    void step();
    void dump();

    u8 alu_xor(u8 lhs, u8 rhs);
    u8 alu_inc(u8 value);
    u8 alu_dec(u8 value);
    u8 alu_sub(u8 lhs, u8 rhs);
    u8 alu_add(u8 lhs, u8 rhs);

    u8 bit_test(u8 value, u8 bit);
    u8 bit_rl(u8 value, bool accum);

    void op_cb();
    void op_jr(Condition condition, i8 offset);
    void op_call(Condition condition, u16 addr);
    void op_ret(Condition condition);

    void push(u16 value);

    u16 pop();

    u8 a = 0;
    Flags f = 0;
    u8 b = 0;
    u8 c = 0;
    u8 d = 0;
    u8 e = 0;
    u8 h = 0;
    u8 l = 0;
    u16 pc = 0;
    u16 sp = 0;

    std::uint64_t cycles = 0;

    Register16<u8, Flags> af{a, f};
    Register16<u8, u8> bc{b, c};
    Register16<u8, u8> de{d, e};
    Register16<u8, u8> hl{h, l};

    MMU& mmu;
    
};

}