#include <algorithm>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <unordered_map>

enum Dir { kPos, kNeg };
Dir dir(int64_t n) { return n < 0 ? kNeg : kPos; }

std::ostream &operator<<(std::ostream &os, const std::vector<int64_t> &vec) {
    if (vec.empty())
        return std::cout << "{}";
    os << "{" << vec[0];
    for (size_t i = 1; i < vec.size(); ++i) {
        os << "," << vec[i];
    }
    return os << "}";
}

bool is_safe_skip(const std::vector<int64_t> &report, std::optional<size_t> skip) {
    std::optional<int64_t> prev;
    std::optional<Dir> delta;
    for (size_t i = 0; i < report.size(); ++i) {
        if (!skip.has_value() || *skip != i) {
            const int64_t curr = report[i];
            if (prev.has_value()) {
                const int64_t diff = curr - *prev;
                const int64_t abs_diff = std::abs(diff);
                const Dir direction = dir(diff);
                if (abs_diff < 1 || abs_diff > 3)
                    return false;
                if (delta.has_value() && direction != *delta)
                    return false;
                delta = direction;
            }
            prev = curr;
        }
    }
    return true;
}

// This is O(N^2) but oh well
bool is_safe(const std::vector<int64_t> &report, bool dampen = false) {
    if (is_safe_skip(report, std::nullopt))
        return true;
    if (!dampen)
        return false;
    for (size_t i = 0; i < report.size(); ++i) {
        if (is_safe_skip(report, i))
            return true;
    }
    return false;
}

int main() {
    const std::regex num("[0-9]+");
    std::ifstream file("../data/full/02");
    std::string line;
    std::smatch match;
    int64_t part1 = 0;
    int64_t part2 = 0;
    while (std::getline(file, line)) {
        std::vector<int64_t> report;
        std::string::const_iterator iter (line.cbegin());
        while (std::regex_search(iter, line.cend(), match, num)) {
            report.push_back(std::stoi(match[0].str()));
            iter = match.suffix().first;
        }
        // part1 += is_safe(report);
        part2 += is_safe(report, /*dampen*/true);
    }
    std::cout << "Part 1: " << part1 << std::endl;
    std::cout << "Part 2: " << part2 << std::endl;
}
