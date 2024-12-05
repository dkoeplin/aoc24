#include <fstream>
#include <regex>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Maybe.h"
#include "nvl/data/Set.h"
#include "nvl/macros/Aliases.h"

struct Rule {
    I64 before;
    I64 after;
};
struct Compare {
    bool operator()(const I64 a, const I64 b) const {
        static const nvl::Set<I64> kEmpty = {};
        return rules.get_or(b, kEmpty).has(a);
    }

    /// Map from page => all pages that should come before it
    nvl::Map<I64,nvl::Set<I64>> rules;
};

nvl::Maybe<Rule> parse_rule(const std::string &line) {
    static const std::regex pattern ("([0-9]+)\\|([0-9]+)");
    std::smatch match;
    if (std::regex_search(line.cbegin(), line.cend(), match, pattern)) {
        return Rule{std::stoi(match[1].str()), std::stoi(match[2].str())};
    }
    return nvl::None;
}

std::vector<I64> parse_list(const std::string &line) {
    static const std::regex pattern ("([0-9]+)");
    auto iter = line.cbegin();
    std::smatch match;
    std::vector<I64> list;
    while (iter != line.cend()) {
        if (std::regex_search(iter, line.cend(), match, pattern)) {
            list.push_back(std::stoi(match[1].str()));
            iter = match.suffix().first;
        } else {
            iter = line.cend();
        }
    }
    return list;
}

int main() {
    std::ifstream file("../data/full/05");
    std::string line;
    Compare compare;
    nvl::List<std::vector<I64>> lists;
    while (std::getline(file, line)) {
        if (auto rule = parse_rule(line)) {
            compare.rules[rule->after].insert(rule->before);
        } else if (!line.empty()) {
            lists.push_back(parse_list(line));
        }
    }

    I64 part1 = 0;
    I64 part2 = 0;
    for (auto &list : lists) {
        if (std::ranges::is_sorted(list, compare)) {
            part1 += list[list.size() / 2];
        } else {
            std::ranges::stable_sort(list, compare);
            part2 += list[list.size() / 2];
        }
    }

    std::cout << "Part 1: " << part1 << std::endl;
    std::cout << "Part 2: " << part2 << std::endl;
}