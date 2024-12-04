#include <fstream>
#include <iostream>
#include <regex>

struct Pattern {
    std::regex regex;
    std::function<void(std::smatch)> func;
};
void parse(const std::string &line, const std::vector<Pattern> &patterns) {
    std::smatch match;
    std::string::const_iterator iter (line.cbegin());
    while (iter != line.cend()) {
        std::optional<long> first_idx = std::nullopt;
        std::optional<size_t> first_pos = std::nullopt;
        std::optional<std::smatch> first_match = std::nullopt;
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (std::regex_search(iter, line.cend(), match, patterns[i].regex)) {
                const auto idx = match.position();
                if (!first_idx.has_value() || idx < *first_idx) {
                    first_idx = idx;
                    first_pos = i;
                    first_match = match;
                }
            }
        }
        if (first_match.has_value()) {
            auto &func = patterns.at(*first_pos).func;
            func(*first_match);
            iter = first_match->suffix().first;
        } else {
            iter = line.cend();
        }
    }
}

int main() {
    const std::regex mul_regex ("mul\\(([0-9]{1,3}),([0-9]{1,3})\\)");
    const std::regex do_regex("do\\(\\)");
    const std::regex dont_regex("don't\\(\\)");

    int64_t part1 = 0;
    int64_t part2 = 0;
    bool enabled = true;
    std::vector<Pattern> patterns;
    patterns.emplace_back(mul_regex, [&](const std::smatch &match) {
        const int64_t mul = std::stoi(match[1].str()) * std::stoi(match[2].str());
        part1 += mul;
        part2 += enabled * mul;
    });
    patterns.emplace_back(do_regex, [&](const auto &){ enabled = true; });
    patterns.emplace_back(dont_regex, [&](const auto &){ enabled = false; });

    std::ifstream file("../data/full/03");
    std::string line;
    while (std::getline(file, line)) {
        parse(line, patterns);
    }

    std::cout << "Part 1: " << part1 << std::endl;
    std::cout << "Part 2: " << part2 << std::endl;
}
