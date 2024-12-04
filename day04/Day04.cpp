#include <fstream>

#include "nvl/data/Tensor.h"
#include "nvl/data/Set.h"

constexpr nvl::Pos<2> directions[8] {
    {-1, -1},
    {-1, 0},
    {-1, 1},
    {0, -1},
    {0, 1},
    {1, -1},
    {1, 0},
    {1, 1}
};

int64_t part1(const nvl::Tensor<2,char> &tensor) {
    const std::string pattern = "XMAS";
    int64_t n = 0;
    for (nvl::Pos<2> idx : tensor.indices()) {
        for (auto delta : directions) {
            bool match = true;
            for (size_t d = 0; match && d < pattern.size(); ++d) {
                match = tensor.get_or(idx + (delta * d), '.') == pattern[d];
            }
            n += match;
        }
    }
    return n;
}

struct Cross {
    static nvl::Set<char> get(const nvl::Tensor<2,char> &tensor, const nvl::Pos<2> &idx, const nvl::List<nvl::Pos<2>> &deltas) {
        nvl::Set<char> result;
        for (auto d : deltas)
            result.insert(tensor.get_or(idx + d, '.'));
        return result;
    }

    explicit Cross(const nvl::Tensor<2,char> &tensor, const nvl::Pos<2> &idx) {
        static const nvl::Set<char> kMatch = {'M', 'S'};
        static const nvl::List<nvl::Pos<2>> l0 = {{-1, -1}, {1, 1}};
        static const nvl::List<nvl::Pos<2>> l1 = {{-1, 1}, {1, -1}};
        if (tensor[idx] != 'A')
            return;
        const auto s0 = get(tensor, idx, l0);
        const auto s1 = get(tensor, idx, l1);
        matched = (s0 == kMatch && s1 == kMatch);
    }
    explicit operator bool() const { return matched; }
    bool matched = false;
};

int64_t part2(const nvl::Tensor<2,char> &tensor) {
    int64_t n = 0;
    for (auto idx : tensor.indices()) {
        n += Cross(tensor, idx).matched;
    }
    return n;
}

int main() {
    nvl::Tensor<2,char> m = nvl::matrix_from_file("../data/full/04");
    std::cout << "Part 1: " << part1(m) << std::endl;
    std::cout << "Part 2: " << part2(m) << std::endl;
}
