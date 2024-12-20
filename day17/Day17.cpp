#include <fstream>
#include <regex>
#include <utility>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/geo/Tuple.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"
#include "nvl/math/Bitwise.h"
#include "nvl/reflect/CastableShared.h"

using namespace nvl;

// 2,4: B = A % 8
// 1,2: B = B xor 2
// 7,5: C = A / 2^B
// 1,3: B = B ^ 3
// 4,4: B = B ^ C
// 5,5: output(B mod 8)
// 0,3: A = A >> 3
// 3,0: repeat

// C = A >> ((A % 8) xor 2)
// B = ((A % 8) xor 2 xor 3) xor (A >> ((A % 3) xor 2))
// A = A >> 3
//

enum OpCode {
    adv = 0, // A = A / 2^combo
    bxl = 1, // B = B xor literal
    bst = 2, // B = combo % 8
    jnz = 3, // pc = A ? literal : pc
    bxc = 4, // B = B xor C
    out = 5, // outputs (combo % 8)
    bdv = 6, // B = A / 2^combo
    cdv = 7  // C = A / 2^combo
};

struct Bit;
struct AbstractBit : CastableShared<Bit, AbstractBit>::BaseClass {
    class_tag(AbstractBit);
    pure virtual std::string to_string() const = 0;
};
struct Bit final : CastableShared<Bit, AbstractBit> {
    using CastableShared::CastableShared;
    using CastableShared::get;
};
struct Reg final : AbstractBit {
    class_tag(Reg, AbstractBit);
    explicit Reg(std::string  name) : name(std::move(name)) {}
    pure std::string to_string() const override { return name; }
    std::string name;
};
struct Lit final : AbstractBit {
    class_tag(Lit, AbstractBit);
    explicit Lit(const bool v) : value(v) {}
    pure std::string to_string() const override { return value ? "1" : "0"; }
    bool value = false;
};
struct Addr final : AbstractBit {
    class_tag(Addr, AbstractBit);
    explicit Addr(Bit a, const U64 idx) : a(std::move(a)), i(idx) {}
    pure std::string to_string() const override { return a->to_string() + "[" + std::to_string(i) + "]"; }
    Bit a;
    U64 i = 0; // 0 = LSBit, 63 = MSBit
};
struct BitNot final : AbstractBit {
    class_tag(BitNot, AbstractBit);
    explicit BitNot(Bit a) : a(std::move(a)) {}
    pure std::string to_string() const override { return "~" + a->to_string(); }
    Bit a;
};
struct BitXor final : AbstractBit {
    class_tag(BitXor, AbstractBit);
    explicit BitXor(Bit a, Bit b) : a(std::move(a)), b(std::move(b)) {}
    pure std::string to_string() const override { return a->to_string() + "x" + b->to_string(); }
    Bit a;
    Bit b;
};

Bit bnot(Bit a) {
    if (auto *lit = a.dyn_cast<Lit>())
        return Bit::get<Lit>(!lit->value);
    if (auto *anot = a.dyn_cast<BitNot>())
        return anot->a;
    return Bit::get<BitNot>(a);
}

// 0 0 | 0
// 0 1 | 1
// 1 0 | 1
// 1 1 | 0
Bit bxor(Bit a, Bit b) {
    if (auto *lit_b = b.dyn_cast<Lit>()) {
        return lit_b->value ? bnot(a) : a;
    } else if (auto *lit_a = a.dyn_cast<Lit>()) {
        return lit_a->value ? bnot(b) : b;
    }
    return Bit::get<BitXor>(a, b);
}


pure List<Bit> padded(const List<Bit> &bits, const U64 len) {
    List<Bit> result;
    if (len > bits.size()) {
        const U64 pad = len - bits.size();
        for (U64 i = 0; i < pad; ++i) {
            result.push_back(Bit::get<Lit>(false));
        }
    }
    result.append(bits);
    return result;
}


List<Bit> shift_right(const List<Bit> &bits, const U64 n) {
    List<Bit> result;
    for (U64 i = 0; i < bits.size() - n; ++i) {
        result.push_back(bits[i]);
    }
    return result;
}

pure List<Bit> inner(List<Bit> bits, const U64 len) {
    bits = padded(bits, len);
    List<Bit> result;
    for (U64 i = bits.size() - len; i < bits.size(); ++i) {
        result.push_back(bits[i]);
    }
    return result;
}

List<Bit> bits_xxor(List<Bit> a, List<Bit> b) {
    const U64 len = std::max(a.size(), b.size());
    a = padded(a, len);
    b = padded(b, len);
    List<Bit> result;
    for (U64 i = 0; i < len; ++i) {
        result.push_back(bxor(a[i], b[i]));
    }
    return result;
}

struct Num;
struct AbstractNum : CastableShared<Num, AbstractNum>::BaseClass {
    class_tag(AbstractNum);
    pure virtual std::string to_string() const = 0;
    pure virtual Num xxor(const Num &rhs) const;
    pure virtual Num mod8() const;
    pure virtual Num div_pow2(const Num &rhs) const;
};
struct Num final : CastableShared<Num, AbstractNum> {
    using CastableShared::CastableShared;
    using CastableShared::get;
};
struct NumXor final : AbstractNum {
    class_tag(NumXor, AbstractNum);
    explicit NumXor(Num a, Num b) : a(std::move(a)), b(std::move(b)) {}
    pure std::string to_string() const override { return "Xor(" + a->to_string() + "," + b->to_string() + ")"; }
    Num a;
    Num b;
};
struct Mod8 final : AbstractNum {
    class_tag(Mod8, AbstractNum);
    explicit Mod8(Num a) : a(std::move(a)) { }
    pure std::string to_string() const override { return "Mod8(" + a->to_string() + ")"; }
    Num a;
};
struct DivPow2 final : AbstractNum {
    class_tag(DivPow2, AbstractNum);
    explicit DivPow2(Num a, Num b) : a(std::move(a)), b(std::move(b)) { }
    pure std::string to_string() const override { return "DivPow2(" + a->to_string() + "," + b->to_string() + ")"; }
    Num a;
    Num b;
};

pure Num AbstractNum::xxor(const Num &rhs) const { return Num::get<NumXor>(self(), rhs); }
pure Num AbstractNum::mod8() const { return Num::get<Mod8>(self()); }
pure Num AbstractNum::div_pow2(const Num &rhs) const { return Num::get<DivPow2>(self(), rhs); }

Maybe<U64> constant(const Num &x);

struct Bits final : AbstractNum {
    class_tag(Bits, AbstractNum);
    explicit Bits(const List<Bit> &bits) : bits(bits) {}

    pure std::string to_string() const override {
        std::stringstream ss;
        ss << bits;
        return ss.str();
    }

    pure Num nnot() const {
        List<Bit> result;
        for (U64 i = 0; i < bits.size(); ++i) {
            result.push_back(bnot(bits[i]));
        }
        return Num::get<Bits>(result);
    }

    pure Num xxor(const Num &rhs) const override {
        if (auto *brhs = rhs.dyn_cast<Bits>()) {
            return Num::get<Bits>(bits_xxor(bits, brhs->bits));
        }
        return AbstractNum::xxor(rhs);
    }

    pure Num mod8() const override { return Num::get<Bits>(inner(bits, 3)); }

    pure Num div_pow2(const Num &rhs) const override {
        if (auto v = constant(rhs)) {
            return Num::get<Bits>(shift_right(bits, *v));
        }
        return AbstractNum::div_pow2(rhs);
    }

    pure U64 len() const { return bits.size(); }

    pure Bit operator[](const U64 i) const { return bits[i]; }

    List<Bit> bits;
};

std::ostream &operator<<(std::ostream &os, const Bit &op) { return os << op->to_string(); }
std::ostream &operator<<(std::ostream &os, const Num &num) { return os << num->to_string(); }

Num named(const std::string &name) {
    Bit reg = Bit::get<Reg>(name);
    List<Bit> bits;
    for (U64 i = 0; i < 64; ++i) {
        bits.push_back(Bit::get<Addr>(reg, 63 - i));
    }
    return Num::get<Bits>(bits);
}

Num lit(const U64 x) {
    List<Bit> bits;
    const U64 len = std::max(1, std::bit_width(x));
    for (U64 i = 0; i < len; ++i) {
        bits.push_back(Bit::get<Lit>((x & (1 << (len - i - 1))) != 0));
    }
    return Num::get<Bits>(bits);
}

Maybe<U64> constant(const Num &x) {
    U64 v = 0;
    if (auto *bits = x.dyn_cast<Bits>()) {
        for (U64 i = 0; i < bits->len(); ++i) {
            if (const auto *lit = (*bits)[i].dyn_cast<Lit>()) {
                v |= lit->value * (1 << (bits->len() - i - 1));
            } else {
                return None;
            }
        }
    }
    return v;
}

struct State {
    static State parse(std::fstream &file) {
        static const std::regex register_regex ("Register (.): (-?[0-9]+)");
        static const std::regex program_regex ("(-?[0-9]+)");
        std::string line;
        std::smatch match;
        State state;
        while (std::getline(file, line)) {
            if (std::regex_search(line.cbegin(), line.cend(), match, register_regex)) {
                const U64 addr = match[1].str()[0] - 'A';
                state.reg[addr] = std::stoll(match[2].str());
            } else if (!line.empty()) {
                auto iter = line.cbegin();
                while (iter != line.cend()) {
                    if (std::regex_search(iter, line.cend(), match, program_regex)) {
                        state.program.push_back(std::stoll(match[1].str()));
                        iter = match.suffix().first;
                    } else {
                        iter = line.cend();
                    }
                }
            }
        }
        return state;
    }

    State() {
        breg[0] = named("A");
        breg[1] = lit(0);
        breg[2] = lit(0);
    }

    pure U64 combo(const U64 x) const {
        return_if(x <= 3, x);
        return_if(x <= 6, reg[x - 4]);
        UNREACHABLE;
    }
    pure Num bcombo(const U64 x) const {
        return_if(x <= 3, lit(x));
        return_if(x <= 6, breg[x - 4]);
        UNREACHABLE;
    }
    pure std::string scombo(const U64 x) const {
        return_if(x <= 3, std::to_string(x));
        return_if(x <= 6, "(" + sreg[x - 4] + ")");
        UNREACHABLE;
    }

    /// Returns false if halt, true otherwise.
    bool inst() {
        return_if(pc >= program.size() - 1, false);

        const U64 op = program[pc] & 0x7;
        const U64 arg = program[pc + 1] & 0x7;
        bool jumped = false;

        switch (op) {
        case adv: {
            reg[0] = reg[0] / (1 << combo(arg));
            if (symbolic) {
                breg[0] = breg[0]->div_pow2(bcombo(arg));
                sreg[0] = "(" + sreg[0] + ") >> " + scombo(arg);
            }
        } break;
        case bdv: {
            reg[1] = reg[0] / (1 << combo(arg));
            if (symbolic) {
                breg[1] = breg[0]->div_pow2(bcombo(arg));
                sreg[1] = "(" + sreg[0] + ") >> " + scombo(arg);
            }
        } break;
        case cdv: {
            reg[2] = reg[0] / (1 << combo(arg));
            if (symbolic) {
                breg[2] = breg[0]->div_pow2(bcombo(arg));
                sreg[2] = "(" + sreg[0] + ") >> " + scombo(arg);
            }
        } break;
        case bxl: {
            reg[1] = reg[1] ^ arg;
            if (symbolic) {
                breg[1] = breg[1]->xxor(lit(arg));
                sreg[1] = "(" + sreg[1] + ")x" + std::to_string(arg);
            }
        } break;
        case bst: {
            reg[1] = combo(arg) & 0x7;
            if (symbolic) {
                breg[1] = bcombo(arg)->mod8();
                sreg[1] = scombo(arg) + "%8";
            }
        } break;
        case bxc: {
            reg[1] = reg[1] ^ reg[2];
            if (symbolic) {
                breg[1] = breg[1]->xxor(breg[2]);
                sreg[1] = sreg[1] + "x" + sreg[2];
            }
        } break;
        case jnz: {
            pc = reg[0] ? arg : pc;
            jumped = reg[0] != 0; // Don't increment the pc on jump
        } break;
        case out: {
            output.push_back(combo(arg) & 0x7);
            if (symbolic) {
                boutput.push_back(bcombo(arg)->mod8());
                soutput.push_back(scombo(arg) + "%8");
            }
        } break;
        default:
            UNREACHABLE;
        }
        pc += jumped ? 0 : 2;
        return true;
    }

    pure std::string str() const {
        std::stringstream ss;
        ss << output.front();
        for (U64 i = 1; i < output.size(); ++i) {
            ss << "," << output[i];
        }
        return ss.str();
    }

    bool run() {
        U64 ops = 0;
        while (inst() && ops < 1'000) { ++ops; }
        if (ops >= 1'000) {
            std::cout << "Timeout!" << std::endl;
        }
        return output == program;
    }

    U64 pc = 0;
    U64 reg [3] = {0};
    Num breg [3];
    std::string sreg [3] = {"A", "0", "0"};

    List<U64> program;
    List<U64> output;
    List<Num> boutput;
    List<std::string> soutput;
    bool symbolic = true;
};

bool solve(U64 &value, List<Bit> lhs, List<Bit> rhs) {
    const U64 len = lhs.size();
    rhs = inner(rhs, len);
    for (U64 b = 0; b < len; ++b) {
        std::cout << "  " << lhs[b]->to_string() << " = " << rhs[b]->to_string() << std::endl;
        const bool lhs_v = lhs[b].dyn_cast<Lit>()->value;
        if (auto *bit = rhs[b].dyn_cast<Addr>()) {
            if (auto *reg = bit->a.dyn_cast<Reg>(); reg && reg->name == "A") {
                value |= lhs_v * (1 << bit->i);
            }
        }
    }
    return true;
}
bool solve(U64 &value, const Num &lhs, const Num &rhs) {
    auto *bits_lhs = lhs.dyn_cast<Bits>();
    auto *bits_rhs = rhs.dyn_cast<Bits>();
    if (bits_lhs && bits_rhs) {
        return solve(value, bits_lhs->bits, bits_rhs->bits);
    }
    return false;
}

Maybe<std::pair<Bits *, Num>> find_bits(Num a, Num b) {
    Bits *bits = nullptr;
    return_if((bits = a.dyn_cast<Bits>()), Some(std::make_pair(bits, b)));
    return_if((bits = b.dyn_cast<Bits>()), Some(std::make_pair(bits, a)));
    return None;
}

void untangle(Num &lhs, Num &rhs) {
    bool changed = true;
    while (changed) {
        changed = false;
        if (const auto *mod8 = rhs.dyn_cast<Mod8>()) {
            if (const auto *nxor = mod8->a.dyn_cast<NumXor>()) {
                //     lhs = mod8(xor(bits, b))
                // ==> xor(lhs, not(bits)) = mod8(b)
                if (const auto pair = find_bits(nxor->a, nxor->b)) {
                    lhs = lhs->xxor(pair->first->mod8());
                    rhs = pair->second->mod8();
                    changed = true;
                }
            }
        }
    }
}

struct Result {
    static Result Pass() { return {}; }
    static Result Fail(const std::string &message) {
        Result result;
        result.success = false;
        result.message = message;
        return result;
    }

    explicit operator bool() { return success; }
    std::string message;
    bool success = true;
};

struct Work {
    static constexpr U64 kShiftPerIter = 3;
    U64 i = 0;
    U64 inner = 0;
    Map<U64, bool> bits;

    Result add_at(const U64 shift, const U64 v) {
        for (U64 b = 0; b < 3; ++b) {
            const U64 idx = shift + i*kShiftPerIter + b;
            const bool bit = (v & (1 << b)) != 0;
            if (!bits.has(idx) || bits.at(idx) == bit) {
                bits[idx] = bit;
            } else {
                return Result::Fail("A[" + std::to_string(idx) + "] != " + (bit ? "1" : "0"));
            }
        }
        return Result::Pass();
    }

    void enumerate() {
        for (U64 b = 0; b < 64; ++b) {
            if (bits.has(b)) {
                std::cout << "A[" << b << "] = " << (bits[b] ? "1" : "0") << " ";
            }
        }
        std::cout << std::endl;
    }
};

std::ostream &indented(std::ostream &os, const U64 i) {
    for (U64 x = 0; x < i; ++x) {
        os << "  ";
    }
    return os;
}

Maybe<U64> solve_part2(const State &state) {
    // 2 xor (~A[2], ~A[1], A[0]) = mod8(A >> {A[2], ~A[1], A[0]})
    // 4 xor (~A[5], ~A[4], A[3]) = mod8(A >> {A[2], ~A[1], A[0]} + {A[5], ~A[4], A[3]})
    List<Work> frontier { {} };
    while (!frontier.empty()) {
        auto &current = frontier.back();
        if (current.i >= state.program.size()) {
            U64 value = 0;
            for (U64 b = 0; b < 64; ++b) {
                if (current.bits.get_or(b, false)) {
                    value |= (1 << b);
                }
            }
            State attempt = state;
            attempt.reg[0] = value;
            attempt.symbolic = false;
            attempt.run();
            std::cout << "A: " << value << std::endl;
            std::cout << "Program: " << state.program << std::endl;
            std::cout << "Output:  " << attempt.output << std::endl;
            frontier.pop_back();
            if (state.program == attempt.output) {
                return value;
            }
        } else if (current.inner <= 7) {
            // 2^0 => X/1 => shift 0
            // 2^1 => X/2 => shift 1
            // 2^2 => X/4 => shift 2
            const U64 out = state.program[current.i];
            const U64 inner = current.inner;
            current.inner += 1;
            /*std::cout << "----- Testing At " << current.i << "/" << state.program.size()
                      << ": bits " << current.i*3 << " to " << ((current.i+1)*3 - 1)
                      << "-----"
                      << std::endl;
            current.enumerate();*/
            Work next = current;
            next.inner = 0;
            if (auto msg = next.add_at(/*i*3 + */0, inner)) {
                indented(std::cout,current.i) << "[" << current.i << "][Y] inner:" << inner << std::endl;
                indented(std::cout,current.i);
                next.enumerate();
                // B[t+1] = ((A[t] % 8) xor 2 xor 3) xor (A[t] >> ((A[t] % 3) xor 2))
                // A[t+1] = A[t] >> 3
                const U64 shift = (inner ^ 2);
                const U64 lhs = out ^ (inner ^ 1);
                // const U64 shift = 1;
                // const U64 lhs = out;
                if (auto msg2 = next.add_at(/*i*3 + */shift, lhs)) {
                    indented(std::cout,current.i) << "[" << current.i << "][Y] Shift = " << inner << "^2 = " << shift
                        << " (" << next.i*3 + shift << ")"
                        << ", lhs = " << out << "^" << inner << "^1 = " << lhs << std::endl;
                    indented(std::cout,current.i);
                    next.enumerate();
                    next.i += 1;
                    frontier.push_back(next);
                } else {
                    indented(std::cout,current.i) << "[" << current.i << "][X] Shift = "
                        << inner << "^2 = " << shift << " (" << next.i*3 + shift << ")"
                    << ", lhs:" << out << "^" << inner << "^1 = " << lhs << ": " << msg2.message << std::endl;
                }
            } else {
                 indented(std::cout,current.i) << "[" << current.i << "][X] Can't have inner " << inner << ": " << msg.message << std::endl;
            }
        } else {
            frontier.pop_back();
        }
    }
    return None;
}

int main() {
    std::fstream file ("../data/full/17");
    State state = State::parse(file);
    State part1 = state;
    part1.run();
    std::cout << "Part 1: " << part1.str() << std::endl;

    State part2 = state;
    part2.run();
    while (part2.output.size() < part2.program.size()) {
        part2.pc = 0;
        part2.run();
    }
    for (U64 i = 0; i < part2.boutput.size() && i < part2.program.size(); ++i) {
        Num lhs = lit(part2.program[i]);
        Num rhs = part2.boutput[i];
        // std::cout << part2.program[i] << " = " << part2.soutput[i] << std::endl;
        untangle(lhs, rhs);
        std::cout << lhs << " = " << rhs << std::endl;
    }
    Maybe<U64> part2_value = solve_part2(part2);
    std::cout << "Part 2: " << (part2_value.has_value() ? std::to_string(*part2_value) : "?") << std::endl;
}
