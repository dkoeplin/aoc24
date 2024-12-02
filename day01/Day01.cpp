#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <unordered_map>

int64_t part1(std::vector<int64_t> &a, std::vector<int64_t> &b) {
    std::ranges::sort(a);
    std::ranges::sort(b);
    int64_t sum = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += std::abs(a[i] - b[i]);
    }
    return sum;
}

int64_t part2(std::vector<int64_t> &a, std::vector<int64_t> &b) {
    std::unordered_map<int64_t, size_t> n;
    for (int64_t i : b) { n[i] += 1; }
    int64_t sim = 0;
    for (int64_t i : a) { sim += i * n[i]; }
    return sim;
}

int main() {
    const std::regex regex("([0-9]+) +([0-9]+)");

    std::vector<int64_t> a, b;
    std::ifstream file("../data/full/01");
    std::string line;
    std::smatch match;
    while (std::getline(file, line)) {
        if (std::regex_match(line, match, regex)) {
            a.push_back(std::stoi(match[1].str()));
            b.push_back(std::stoi(match[2].str()));
        }
    }
    std::cout << "Part 1: " << part1(a, b) << std::endl;
    std::cout << "Part 2: " << part2(a, b) << std::endl;

    return 0;
}
