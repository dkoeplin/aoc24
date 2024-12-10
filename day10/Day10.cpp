#include <fstream>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/data/Tensor.h"
#include "nvl/geo/Tuple.h"
#include "nvl/time/Clock.h"
#include "nvl/time/Duration.h"

using nvl::List;
using nvl::Map;
using nvl::Set;
using Matrix = nvl::Tensor<2, char>;
using Pos = nvl::Pos<2>;

static constexpr Pos kDirections[4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

struct Ranking {
    Ranking &operator+=(const Ranking &rhs) {
        score += rhs.score;
        rating += rhs.rating;
        return *this;
    }
    U64 score = 0;
    U64 rating = 0;
};

template <>
struct std::hash<List<Pos>> {
    pure U64 operator()(const List<Pos> &list) const noexcept { return nvl::sip_hash(list.range()); }
};

Ranking trailhead_ranking(const Matrix &map, const Pos &trailhead) {
    Set<Pos> ends;
    Set<List<Pos>> paths;
    Map<Pos, U64> next_index;
    List<Pos> path { trailhead };
    while (!path.empty()) {
        const Pos &current = path.back();
        const char height = map[current];
        U64 &next_i = next_index.get_or_add(current, 0);
        if (height == '9') {
            ends.insert(current);
            paths.insert(path);
            next_i = 4;
        }
        if (next_i < 4) {
            const Pos next = current + kDirections[next_i];
            const char next_h = static_cast<char>(height + 1);
            if (map.has(next) && map[next] == next_h) {
                path.push_back(next);
            }
            next_i += 1;
        } else {
            next_index.remove(current);
            path.pop_back();
        }
    }
    return {.score = ends.size(), .rating = paths.size()};
}

Ranking trailhead_ranking(const Matrix &map, const List<Pos> &trailheads) {
    Ranking ranking;
    for (const Pos &trailhead : trailheads) {
        ranking += trailhead_ranking(map, trailhead);
    }
    return ranking;
}

List<Pos> get_trailheads(const Matrix &map) {
    List<Pos> trailheads;
    for (const auto i : map.indices()) {
        if (map[i] == '0') {
            trailheads.push_back(i);
        }
    }
    return trailheads;
}

int main() {
    const Matrix map = nvl::matrix_from_file("../data/full/10");
    const auto start = nvl::Clock::now();
    const List<Pos> trailheads = get_trailheads(map);
    const auto [score, rating] = trailhead_ranking(map, trailheads);
    const auto end = nvl::Clock::now();
    std::cout << "Part 1: " << score << std::endl;
    std::cout << "Part 2: " << rating << std::endl;
    std::cout << "Time: " << nvl::Duration(end - start) << std::endl;
}
