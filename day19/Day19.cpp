#include <fstream>
#include <regex>
#include <iostream>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/macros/Aliases.h"

using namespace nvl;

struct Match {
    U64 offset = 0;
    I64 p = 0;
};

U64 matches(const std::string &line, const List<std::string> &patterns) {
    if (line.empty())
        return {};
    Map<U64, U64> start;
    Map<U64, U64> cache;
    List<Match> frontier{ {0, -1} };
    U64 total = 0;
    while (!frontier.empty()) {
        auto &[i, p] = frontier.back();
        if (p == -1 && i < line.size()) {
            start[i] = total;
        }
        p += 1;
        if (i >= line.size()) {
            frontier.pop_back();
            total += 1;
        } else if (cache.has(i)) {
            frontier.pop_back();
            total += cache[i];
        } else if (p < static_cast<I64>(patterns.size())) {
            const auto &pattern = patterns[p];
            const auto slice = std::string_view(line).substr(i, pattern.size());
            if (slice == pattern) {
                frontier.push_back({i + pattern.size(), -1});
            }
        } else {
            cache[i] = total - start[i];
            frontier.pop_back();
        }
    }
    return total;
}

List<std::string> patterns(const std::string &line) {
    static const std::regex pattern ("([a-z]+)");
    auto iter = line.cbegin();
    List<std::string> patterns;
    while (iter != line.cend()) {
        std::smatch match;
        if (std::regex_search(iter, line.cend(), match, pattern)) {
            patterns.push_back(match[1].str());
            iter = match.suffix().first;
        } else {
            iter = line.end();
        }
    }
    return patterns;
}

int main() {
    std::fstream file ("../data/full/19");
    std::string line;
    std::getline(file, line);
    const List<std::string> towels = patterns(line);
    U64 part1 = 0;
    U64 part2 = 0;
    while (std::getline(file, line)) {
        const auto match = matches(line, towels);
        part1 += (match > 0);
        part2 += match;
    }
    std::cout << "Part 1: " << part1 << std::endl;
    std::cout << "Part 2: " << part2 << std::endl;
}
