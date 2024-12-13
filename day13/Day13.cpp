#include <fstream>
#include <regex>

#include "nvl/data/Maybe.h"
#include "nvl/geo/Tuple.h"

using namespace nvl;

Maybe<I64> div_if(const I64 a, const I64 b) { return SomeIf(a / b, a % b == 0); }

struct Machine {
    static bool parse(Pos<2> &pos, std::fstream &file) {
        static const std::regex regex(".*: X.([0-9]+), Y.([0-9]+)");
        std::string line;
        std::smatch match;
        std::getline(file, line);
        if (std::regex_search(line.cbegin(), line.cend(), match, regex)) {
            pos[0] = std::stoll(match[1].str());
            pos[1] = std::stoll(match[2].str());
            return true;
        }
        return false;
    }
    static Maybe<Machine> parse(std::fstream &file) {
        Machine result;
        if (parse(result.da, file) && parse(result.db, file) && parse(result.p, file)) {
            std::string line;
            std::getline(file, line);
            return Some(std::move(result));
        }
        return None;
    }

    U64 solve() {
        // P = da*a + db*b
        // c = a*3 + b
        // a*Ax + b*Bx = Px
        // a*Ay + b*By = Py
        // b = (Px - a*Ax)/Bx

        // a*Ay + By(Px - aAx)/Bx = Py
        // a(Ay - AxBy/Bx) + ByPx/Bx = Py
        // a = (Py - PxBy/Bx)/(Ay - AxBy/Bx)
        // a = (PyBx - PxBy)/(AyBx - AxBy)
        if (auto a = div_if(p[1]*db[0] - p[0]*db[1], da[1]*db[0] - da[0]*db[1])) {
            if (auto b = div_if(p[0] - *a*da[0], db[0])) {
                return *a * 3 + *b * 1;
            }
        }
        return 0;
    }

    Pos<2> da; // Delta for button A
    Pos<2> db; // Delta for button B
    Pos<2> p;  // Location of prize
};

int main() {
    std::fstream file ("../data/full/13");
    U64 part1 = 0;
    U64 part2 = 0;
    while (auto machine = Machine::parse(file)) {
        part1 += machine->solve();
        machine->p += 10000000000000LL;
        part2 += machine->solve();
    }
    std::cout << "Part 1: " << part1 << std::endl;
    std::cout << "Part 2: " << part2 << std::endl;
}
