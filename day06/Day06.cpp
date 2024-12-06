#include <fstream>

#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/data/SipHash.h"
#include "nvl/data/Tensor.h"
#include "nvl/geo/Tuple.h"
#include "nvl/macros/Pure.h"

static const nvl::Map<char, I64> kChar2Direction {{'<', 0}, {'^', 1}, {'>', 2}, {'v', 3}};

struct Guard {
    static constexpr nvl::Pos<2> kDirections[4] {
        {0, -1}, // Left (y,x)
        {-1, 0}, // Up
        {0, 1},  // Right
        {1, 0}   // Down
    };

    pure Guard move(const nvl::Tensor<2, char> &map) const {
        Guard next = *this;
        next.pos = pos + kDirections[dir];
        if (map.get_or(next.pos, '.') == '#') {
            // Count turning as a single move
            next.pos = pos;
            next.dir = (next.dir + 1) % 4;
        }
        return next;
    }

    pure bool operator==(const Guard &rhs) const { return pos == rhs.pos && dir == rhs.dir; }
    pure bool operator!=(const Guard &rhs) const { return !(*this == rhs); }

    nvl::Pos<2> pos;
    I64 dir;
};

template <>
struct std::hash<Guard> {
    pure U64 operator()(const Guard &guard) const noexcept { return nvl::sip_hash(guard); }
};

pure Guard start(const nvl::Tensor<2,char> &map) {
    for (const auto i : map.indices()) {
        if (auto iter = kChar2Direction.find(map[i]); iter != kChar2Direction.end()) {
            return {i, iter->second};
        }
    }
    ASSERT(false, "No starting location found.");
}

struct WalkResult {
    nvl::Set<Guard> visited;
    nvl::Set<nvl::Pos<2>> unique;
    bool loop = false;
};
WalkResult walk(const nvl::Tensor<2,char> &map, const Guard &start) {
    Guard curr = start;
    WalkResult result;
    while (!result.visited.has(curr) && map.has(curr.pos)) {
        result.visited.insert(curr);
        result.unique.insert(curr.pos);
        curr = curr.move(map);
    }
    result.loop = map.has(curr.pos);
    return result;
}

I64 part2(nvl::Tensor<2, char> &map, const Guard &start, const WalkResult &part1) {
    I64 part2 = 0;
    // Only check positions along the original route.
    nvl::Set<nvl::Pos<2>> candidates;
    for (const auto &g : part1.visited) {
        const Guard next = g.move(map);
        if (next.pos != g.pos && map.get_or(next.pos, '#') == '.') {
            candidates.insert(next.pos);
        }
    }
    for (const auto &pos : candidates) {
        map[pos] = '#';
        part2 += walk(map, start).loop;
        map[pos] = '.';
    }
    return part2;
}

int main() {
    nvl::Tensor<2,char> map = nvl::matrix_from_file("../data/full/06");
    const Guard begin = start(map);
    const WalkResult part1 = walk(map, begin);
    std::cout << "Part 1: " << part1.unique.size() << std::endl;
    std::cout << "Part 2: " << part2(map, begin, part1) << std::endl;
}
