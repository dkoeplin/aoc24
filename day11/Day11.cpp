#include <fstream>
#include <iostream>
#include <list>
#include <regex>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"

using nvl::List;
using nvl::Map;

List<U64> parse_ints(const std::string &filename) {
    static const std::regex num("([0-9]+)");

    std::fstream file (filename);
    std::string line;
    std::getline(file, line);

    List<U64> ints;
    auto iter = line.cbegin();
    std::smatch match;
    while (iter != line.end()) {
        if (std::regex_search(iter, line.cend(), match, num)) {
            ints.push_back(std::stoll(match[1].str()));
            iter = match.suffix().first;
        } else {
            iter = line.end();
        }
    }
    return ints;
}

U64 num_digits(const U64 x) { return static_cast<U64>(std::ceil(std::log10(x + 1))); }

std::pair<U64, U64> split_digits(const U64 num) {
    const U64 n = num_digits(num);
    const U64 d = static_cast<U64>(std::pow(10, n/2));
    const U64 split0 = num / d;
    const U64 split1 = num % d;
    return {split0, split1};
}

struct Stone {
    Stone(const U64 v, const U64 t, const U64 n) : v(v), t(t), n(n) {}
    U64 v;       // Value on stone
    U64 t;       // Time created
    U64 n;       // Number of stones finished when created (ignored when hashing)
    bool visited = false;
};

struct StoneHash {
    pure U64 operator()(const Stone &stone) const noexcept { return nvl::sip_hash(std::pair{stone.v, stone.t}); }
};

struct StoneEq {
    pure bool operator()(const Stone &a, const Stone &b) const noexcept { return a.v == b.v && a.t == b.t; }
};

using Cache = Map<Stone,U64,StoneHash,StoneEq>;

U64 blinks(Cache &cache, const U64 stone, const U64 N) {
    U64 n = 0;
    List<Stone> stones;
    stones.emplace_back(/*v*/stone, /*t*/0, n);
    while (!stones.empty()) {
        Stone &curr = stones.back();
        if (auto iter = cache.find(curr); iter != cache.end()) {
            n += iter->second;
            stones.pop_back();
        } else if (curr.t >= N || curr.visited) {
            if (!curr.visited) {
                curr.n = n;
                n += 1;
            }
            cache[curr] = n - curr.n; // Number finished to the right since this was created
            stones.pop_back();
        } else if (curr.v == 0) {
            curr.visited = true;
            curr.n = n;
            stones.emplace_back(1, curr.t + 1, n);
        } else if (num_digits(curr.v) % 2 == 0) {
            curr.visited = true;
            curr.n = n;
            const U64 t = curr.t;
            const auto [l, r] = split_digits(curr.v);
            stones.emplace_back(/*v*/l, t + 1, n + 1); // Don't count to the right
            stones.emplace_back(/*v*/r, t + 1, n);
        } else {
            curr.visited = true;
            curr.n = n;
            stones.emplace_back(curr.v * 2024, curr.t + 1, n);
        }
    }
    return n;
}

U64 blinks(const List<U64> &stones, const U64 N) {
    Cache cache;
    U64 n = 0;
    for (const U64 stone : stones) {
        n += blinks(cache, stone, N);
    }
    return n;
}

int main() {
    const List<U64> stones = parse_ints("../data/full/11");
    std::cout << "Part 1: " << blinks(stones, 25) << std::endl;
    std::cout << "Part 2: " << blinks(stones, 75) << std::endl;
}
