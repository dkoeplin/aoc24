#include <fstream>
#include <iostream>
#include <regex>

static const std::regex mul_regex ("mul\\(([0-9]{1,3}),([0-9]{1,3})\\)");
static const std::regex do_regex("do\\(\\)");
static const std::regex dont_regex("don't\\(\\)");

struct Pattern {
    std::regex regex;
    std::function<void(std::smatch)> func;
};
void parse(const std::vector<std::string> &lines, const std::vector<Pattern> &patterns) {
    std::smatch match;
    for (const auto &line : lines) {
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

}

int64_t part1(const std::vector<std::string> &lines) {
    int64_t part1 = 0;
    Pattern pattern {.regex = mul_regex, .func = [&](const std::smatch &match) {
        part1 += std::stoi(match[1].str()) * std::stoi(match[2].str());
    }};
    parse(lines, {pattern});
    return part1;
}

int64_t part2(const std::vector<std::string> &lines) {
    int64_t part2 = 0;
    bool enabled = true;
    Pattern mul_pattern {.regex = mul_regex, .func = [&](const std::smatch &match) {
        if (enabled) {
            // Multiplications are skipped if we're in a disabled block
            part2 += std::stoi(match[1].str()) * std::stoi(match[2].str());
        }
    }};
    Pattern do_pattern = {.regex = do_regex, .func = [&](const std::smatch &){ enabled = true; }};
    Pattern dont_pattern = {.regex = dont_regex, .func = [&](const std::smatch &){ enabled = false; }};
    parse(lines, {mul_pattern, do_pattern, dont_pattern});
    return part2;
}

int main() {
    std::ifstream file("../data/full/03");
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    std::cout << "Part 1: " << part1(lines) << std::endl;
    std::cout << "Part 2: " << part2(lines) << std::endl;
}
