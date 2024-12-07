#include <nvl/time/Duration.h>

#include <fstream>
#include <regex>

#include "nvl/data/Counter.h"
#include "nvl/data/List.h"
#include "nvl/data/Maybe.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/time/Clock.h"

U64 num_digits(const U64 x) { return static_cast<U64>(std::ceil(std::log10(x + 1))); }

struct Line {
    static U64 op(const U64 a, const U64 b, const U64 op) {
        switch (op) {
        case 0: return a + b;
        case 1: return a * b;
        case 2: return a * static_cast<U64>(std::pow(10, num_digits(b))) + b;
        default: UNREACHABLE;
        }
    }
    pure bool may_be_true(const U64 num_ops) const {
        const auto counter = nvl::Counter<U64>::get(rhs.size() - 1, num_ops);
        return counter.exists([&](const nvl::List<U64> &ops) {
            U64 total = rhs[0];
            for (size_t i = 0; i < ops.size(); ++i) {
                total = op(total, rhs[i + 1], ops[i]);
            }
            return total == lhs;
        });
    }
    U64 lhs = 0;
    nvl::List<U64> rhs;
};

nvl::Maybe<Line> parse_line(const std::string &line) {
    static const std::regex equation ("([0-9]+):(.*)");
    static const std::regex num ("([0-9]+)");
    std::smatch match;
    if (std::regex_search(line.cbegin(), line.cend(), match, equation)) {
        Line result;
        result.lhs = std::stoll(match[1].str());
        auto iter = match[2].first;
        while (iter != line.end()) {
            if (std::regex_search(iter, line.cend(), match, num)) {
                result.rhs.push_back(std::stoll(match[1].str()));
                iter = match.suffix().first;
            } else {
                iter = line.cend();
            }
        }
        return result;
    }
    return nvl::None;
}

int main() {
    const auto start = nvl::Clock::now();
    std::ifstream file("../data/full/07");
    std::string line;
    U64 part1 = 0;
    U64 part2 = 0;
    while (std::getline(file, line)) {
        if (auto eq = parse_line(line)) {
            part1 += eq->may_be_true(2) * eq->lhs;
            part2 += eq->may_be_true(3) * eq->lhs;
        }
    }
    std::cout << "Part 1: " << part1 << std::endl;
    std::cout << "Part 2: " << part2 << std::endl;
    const auto end = nvl::Clock::now();
    std::cout << "Time: " << nvl::Duration(end - start) << std::endl;
}
